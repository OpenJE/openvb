/**
 * doctor - one-command harness sanity for OpenVB RE sessions
 *
 * Turns the prose in .pi/skills (wine prefix "boot lies", headless server
 * liveness, DXVK impostor dlls, staged resource.rht, ledger/fresh-IDB state)
 * into one deterministic table that crew workers and cold sessions can trust.
 * Everything is local and read-only: no saves, no heals, no devenv mutations.
 * Where a heal exists, the fix column names the command.
 */

import * as fs from "node:fs";
import * as net from "node:net";
import * as path from "node:path";
import { Type } from "typebox";
import type { ExtensionAPI } from "@earendil-works/pi-coding-agent";

type Status = "ok" | "warn" | "fail";

interface Row {
	name: string;
	status: Status;
	detail: string;
	fix?: string;
}

const tcpAlive = (port: number, timeoutMs = 1200): Promise<boolean> =>
	new Promise((resolve) => {
		const sock = net.connect({ host: "127.0.0.1", port });
		const done = (v: boolean) => {
			sock.destroy();
			resolve(v);
		};
		sock.setTimeout(timeoutMs, () => done(false));
		sock.once("connect", () => done(true));
		sock.once("error", () => done(false));
	});

const execQuiet = async (pi: ExtensionAPI, cmd: string, args: string[]): Promise<{ code: number; out: string }> => {
	try {
		const r = await pi.exec(cmd, args, { timeout: 25_000 });
		return { code: r.code ?? 1, out: `${r.stdout}\n${r.stderr}`.trim().slice(0, 4000) };
	} catch (e) {
		return { code: 127, out: e instanceof Error ? e.message : String(e) };
	}
};

export default function (pi: ExtensionAPI) {
	// Extensions run in the pi PROCESS whose cwd can differ from the session
	// workspace; anchor every relative resolution to ctx.cwd from session_start.
	let baseCwd = process.cwd();
	pi.on("session_start", (_e, ctx) => {
		baseCwd = ctx.cwd;
	});
	const R = (p2: string): string => path.resolve(baseCwd, p2);

	const run = async (): Promise<string> => {
		const rows: Row[] = [];
		const env = process.env;

		// 1. headless IDA servers (devenv processes)
		const origPort = Number(env.OPENVB_IDA_ORIG_MCP_PORT || 8745);
		const reimplPort = Number(env.OPENVB_IDA_REIMPL_MCP_PORT || 8746);
		const [origAlive, reimplAlive] = await Promise.all([tcpAlive(origPort), tcpAlive(reimplPort)]);
		rows.push(
			{
				name: "ida-original-headless",
				status: origAlive ? "ok" : "fail",
				detail: `127.0.0.1:${origPort} ${origAlive ? "listening" : "dead"}`,
				fix: origAlive ? undefined : "devenv up (processes.ida-orig-headless)",
			},
			{
				name: "ida-reimpl-headless",
				status: reimplAlive ? "ok" : "warn",
				detail: `127.0.0.1:${reimplPort} ${reimplAlive ? "listening" : "dead (only needed for compare/validation waves)"}`,
			},
		);

		// 2. original IDB + F3.json reference present
		const origIdb = env.OPENVB_ORIG_IDB ? R(env.OPENVB_ORIG_IDB) : R("artifacts/F3.exe.i64");
		let idb: Row = { name: "original-idb", status: "fail", detail: `${origIdb} missing`, fix: "restore from backup / reanalyze - never trust a silent regenerate" };
		try {
			const st = fs.statSync(origIdb);
			const ageDays = (Date.now() - st.mtimeMs) / 86_400_000;
			idb = {
				name: "original-idb",
				status: "ok",
				detail: `${(st.size / 1_048_576).toFixed(0)}MB, mtime ${ageDays.toFixed(1)}d ago`,
			};
		} catch {
			/* keep fail row */
		}
		rows.push(idb);
		const f3json = R("artifacts/F3.json");
		rows.push({
			name: "f3-json-frozen-ref",
			status: fs.existsSync(f3json) ? "ok" : "fail",
			detail: fs.existsSync(f3json) ? "present (frozen 2024-04-04 OOAnalyzer export)" : "missing",
			fix: fs.existsSync(f3json) ? undefined : "restore - rung-1 comparison basis",
		});

		// 3. ledgers: recovery evidence from guard/ida saves
		const ledgerDir = R(env.OPENVB_LEDGER_DIR || "artifacts/ledgers");
		let ledgerRow: Row = { name: "idb-ledgers", status: "warn", detail: `${ledgerDir} missing - no EA->name recovery source yet` };
		try {
			const files = fs.readdirSync(ledgerDir).filter((f) => f.endsWith(".jsonl"));
			const newest = [...files].sort().pop(); // never sort().pop() the tested array - pop mutates it
			ledgerRow = {
				name: "idb-ledgers",
				status: files.length ? "ok" : "warn",
				detail: files.length ? `${files.length} files, newest ${newest}` : "dir empty - run ida save",
			};
		} catch {
			/* keep warn */
		}
		rows.push(ledgerRow);

		// 4. ghidra second decompiler config
		const ghidraCfg = path.join(process.env.PI_CODING_AGENT_DIR || path.join(process.env.HOME || "/home/shane", ".pi", "agent"), "pi-ghidra.json");
		let ghStatus: Status = "warn";
		let ghDetail = "no pi-ghidra.json - ghidra tool will ask for setup";
		try {
			const cfg = JSON.parse(fs.readFileSync(ghidraCfg, "utf8"));
			const home = cfg?.ghidraHome;
			if (home && fs.existsSync(home)) {
				ghStatus = "ok";
				ghDetail = `configured: ${home}`;
			} else if (home) {
				ghDetail = `configured path GONE (nix GC?): ${home}`;
				ghStatus = "fail";
			}
		} catch {
			/* keep warn */
		}
		rows.push({
			name: "ghidra-home",
			status: ghStatus,
			detail: ghDetail,
			fix: ghStatus === "fail" ? "ghidra action=setup with current store path, or /ghidra-setup" : undefined,
		});

		// 5. wine prefix triad (only when prefixes exist)
		const prefixes = ["f3-reimpl", "f3demo"]
			.map((p) => R(`.devenv/state/${p}-wineprefix`))
			.filter((p) => fs.existsSync(p));
		if (prefixes.length) {
			const dx = await execQuiet(pi, "f3-wine-dxvk", ["status"]);
			const verdictLine = dx.out.split("\n").find((l) => /verdict:/.test(l)) || "";
			const verdict = /ARMED/.test(verdictLine) ? "ok" : /UNKNOWN|IMPOSTOR|PARTIAL/.test(verdictLine) ? "fail" : "warn";
			rows.push({
				name: "dxvk-d3d8",
				status: dx.code === 0 ? (verdict as Status) : "warn",
				detail: verdictLine.trim().slice(0, 200) || "f3-wine-dxvk status produced no verdict line",
				fix: dx.code === 0 && (verdict === "fail" || verdict === "warn") ? "f3-wine-dxvk ensure <prefix> - idempotent re-arm from THIS host `s declared dxvk; run scripts self-heal now" : undefined,
			});
			const user = env.USER || env.LOGNAME || "shane";
			for (const p of prefixes) {
				const docs = path.join(p, `drive_c/users/${user}/Documents`);
				let dangling = false;
				try {
					dangling = fs.existsSync(docs) && !fs.existsSync(fs.realpathSync(docs));
				} catch {
					dangling = true;
				}
				if (dangling) rows.push({ name: `wine-userdirs:${p.split("/").pop()}`, status: "fail", detail: "Documents link dangles - save-folder fatals (boot lie #1)", fix: "f3-wine-userdirs-fix <prefix>" });
			}
		} else {
			rows.push({ name: "wine-prefixes", status: "warn", detail: "no prefixes under .devenv/state - created by first f3-*-run", fix: "run f3-reimpl-run or f3demo-run" });
		}

		// 6. staged data dir (boot lie #2: resource.rht in cwd)
		const rht = R(".devenv/state/f3-reimpl-data/resource.rht");
		rows.push({
			name: "staged-resource-rht",
			status: fs.existsSync(rht) ? "ok" : "warn",
			detail: fs.existsSync(rht) ? "f3-reimpl-data staged" : "not staged - bare f3-reimpl-run does this; runs via *-data scripts handle it",
		});

		// 7. reimpl exe freshness vs IDB (rebuild rolls the reimpl IDB; stale MCP server after rebuild)
		const exe = R(env.OPENVB_REIMPL_EXE || "build/F3.exe");
		const reimplIdb = R(env.OPENVB_REIMPL_IDB || "build/F3.exe.i64");
		if (fs.existsSync(exe) && fs.existsSync(reimplIdb)) {
			const e = fs.statSync(exe).mtimeMs;
			const i = fs.statSync(reimplIdb).mtimeMs;
			rows.push({
				name: "reimpl-idb-vs-exe",
				status: i >= e ? "ok" : "warn",
				detail: i >= e ? "IDB not older than exe" : "reimpl IDB older than rebuilt exe - refresh ida_reimplementation_openvb via its documented flow",
			});
		}

		const fmt = (r: Row) => `${r.status === "ok" ? "[ok]  " : r.status === "warn" ? "[warn]" : "[FAIL]"} ${r.name}: ${r.detail}${r.fix ? `  -> fix: ${r.fix}` : ""}`;
		const fails = rows.filter((r) => r.status === "fail").length;
		const warns = rows.filter((r) => r.status === "warn").length;
		return `harness doctor - ${rows.length - fails - warns}/${rows.length} ok\n${rows.map(fmt).join("\n")}`;
	};

	pi.registerTool({
		name: "doctor",
		label: "Doctor",
		description: "Read-only harness sanity: IDA servers, original IDB + F3.json, ledgers, ghidra home, wine prefix triad (dxvk/userdirs/rht), reimpl IDB freshness. Run at session start or before any build/run/compare wave.",
		promptSnippet: "One read-only harness health check with fix hints (run before waves; at session start).",
		promptGuidelines: [
			"Run doctor once at session start and before build/run/compare waves; act on [FAIL] rows' fix commands, report [warn] only when it gates the current task.",
		],
		parameters: Type.Object({}),
		async execute() {
			return { content: [{ type: "text", text: await run() }] };
		},
	});

	pi.registerCommand("doctor", {
		description: "harness sanity table",
		handler: async (_args, ctx) => {
			ctx.ui.notify(await run(), "info");
		},
	});
}
