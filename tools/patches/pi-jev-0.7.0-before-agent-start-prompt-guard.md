pi-jev@0.7.0 — before_agent_start prompt guard (2026-10-23)

Crash: Extension ".../pi-jev/extensions/index.ts" error: Cannot read properties of
undefined (reading 'trim') at classifyThinkingNeed (src/thinking.ts:45) fired on every
emitBeforeAgentStart whose event carried no prompt (sendCustomMessage runs, some
extension-emitted events).

Fix (applied in place at .pi/npm/node_modules/pi-jev/extensions/index.ts): the
before_agent_start handler normalizes once at the boundary —

    const prompt = typeof event.prompt === "string" ? event.prompt : "";

— and every call site in the handler (agents.dispatch regex, promptHasUrl,
autoModel.plan, autoModel.judge, auto.route, autoThinking.route x2) uses the
normalized string instead of event.prompt.

DURABILITY: the .pi/* tree is gitignored and pi-jev@0.7.0 is the newest published
version (checked 2026-10-23), so this patch exists only in the installed copy. Any
`npm install` that reinstalls pi-jev REVERTS it. Re-apply by editing the handler as
above (7 lines), or upstream it (the classifier boundary belongs in pi-jev itself).

Verified: pi -ne -e .pi/npm/node_modules/pi-jev/extensions/index.ts -p --no-session "hi"
loads clean (the error previously fired at every before_agent_start); guard semantics
checked directly (undefined -> "", " hi " -> "hi").
