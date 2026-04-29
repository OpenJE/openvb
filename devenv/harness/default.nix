# ./devenv/harness/default.nix

{ lib, ... }: {
  imports = [
    ./permissions
    ./mcp-servers
    ./agents
    ./skills
    ./commands
  ];

  opencode = {
    enable = true;

    settings = {
      provider = {
        lmstudio = {
          npm = "@ai-sdk/openai-compatible";
          name = "LM Studio";

          options = {
            baseURL = "http://127.0.0.1:1234/v1";
          };

          models = {
            "qwen3.6-35b-a3b@q4_k_xl" = {
              name = "Qwen3.6 35B A3B (q4_k_xl)";
            };
          };
        };
      };

      compaction = {
        auto = true;
        prune = true;
      };

      plugin = [
        "opencode-lmstudio@0.3.0"
        "@tarquinen/opencode-dcp@3.1.5"
      ];

      agent = {
        #plan.disable = true;
        #build.disable = true;
        #general.disable = true;
        #explore.disable = true;
      };

      permission = {
        bash = {
          "*" = "ask";
          "devenv*" = "allow";
          "git status*" = "allow";
          "git diff*" = "allow";
          "git log*" = "allow";
          "nix eval*" = "allow";
          "nix path-info*" = "allow";
          "nix shell*" = "ask";
          "docker*" = "ask";
        };
      };
    };

    rules = lib.strings.removeSuffix "\n" ''
      # OpenVB Reverse Engineering Rules

      - Treat this repository as a reverse-engineering and faithful reimplementation project for Fallout: Van Buren.
      - Treat the original demo and rebuilt reimplementation as two explicit analysis/debug targets that can be compared at multiple fidelity levels.
      - The canonical original database is `./F3.exe.i64` and the canonical reimplementation database path is `./.devenv/state/ida/F3-reimpl.i64`.
      - Keep the built-in `Plan` and `Build` agents available for general planning and implementation work.
      - Use `overseer` as the primary user-facing reverse-engineering agent.
      - Use `pathfinder` as the internal recursive traversal engine for deep IDA-backed function analysis.
      - Use the headless orig/reimpl IDA MCP servers rather than assuming a live GUI plugin session.
      - Open the intended original or reimplementation database explicitly before analyzing, comparing, debugging, or curating.
      - Prefer evidence from the codebase, docs, and IDA MCP over inference or guesswork.
      - When analyzing binary behavior, separate confirmed findings from hypotheses and open questions.
      - Escalate comparison from binary surface to structure, types, pseudocode, and runtime checkpoints only when needed.
      - Use IDA MCP tools to inspect decompilation, disassembly, xrefs, types, and comments before proposing behavior changes.
      - Safe metadata curation in the repo-specific reimplementation IDA database is allowed; binary patching and arbitrary IDAPython are not default workflow steps.
      - Treat function analysis as bounded graph traversal with memoized summaries, not unbounded recursive delegation.
      - Detect repeated nodes, recursion, and SCC-like cycles before descending further.
      - Use bounded verification loops to refine summaries before propagating conclusions upward.
      - Keep the reimplementation aligned with observed binary behavior rather than rewriting for cleanliness.
      - Update repository documentation such as `docs/f3/*` when analysis produces durable findings.
      - Preserve the existing Windows/MSVC build assumptions and avoid introducing unrelated tooling changes.
    '';
  };
}
