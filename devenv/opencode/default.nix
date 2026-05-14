# ./devenv/opencode/default.nix

{ config, lib, ... }: {
  imports = [
    ./agent
    ./command
    ./mcp
    ./permission
    ./plugin
    ./skill
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
            "qwen3.6-27b" = {
              name = "Qwen 3.6 27B";
            };

            "qwen3.6-35b-a3b" = {
              name = "Qwen 3.6 35B A3B";
            };

            "glm-4.7-flash" = {
              name = "GLM 4.7 Flash";
            };

            "gemma-4-31b-it" = {
              name = "Gemma 4 31B IT";
            };

            "nvidia-nemotron-3-nano-omni-30b-a3b-reasoning" = {
              name = "Nvidia Nemotron 3 Nano Omni 30B A3B Reasoning";
            };
          };
        };
      };

      compaction = {
        auto = true;
        prune = true;
      };

      plugin = [
        "oh-my-openagent@4.0.0"
        "file:${config.devenv.root}/modules/opencode-openje"
      ];
    };

    rules = lib.strings.removeSuffix "\n" ''
      # OpenVB Reverse Engineering Rules

      - Treat this repository as a reverse-engineering and faithful reimplementation project for Fallout: Van Buren.
      - Treat the original demo and rebuilt reimplementation as two explicit analysis/debug targets that can be compared at multiple fidelity levels.
      - The canonical original database is `./F3.exe.i64` and the canonical reimplementation database path is `./.devenv/state/ida/F3-reimpl.i64`.
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
