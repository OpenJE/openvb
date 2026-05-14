# ./devenv/opencode/plugin/default.nix

{ ... }: {
  files.".opencode/oh-my-openagent.jsonc".text = builtins.toJSON {
    "$schema" = "https://raw.githubusercontent.com/code-yeongyu/oh-my-openagent/dev/assets/oh-my-opencode.schema.json";
    team_mode = {
      enabled = true;
      max_parallel_members = 4;
      max_members = 8;
      tmux_visualization = false;
    };
    agents = {
      # Main RE orchestrator / lead.
      sisyphus.model = "lmstudio/qwen3.6-27b";

      # Code-aware planning and decomposition.
      prometheus.model = "lmstudio/qwen3.6-27b";
      metis.model = "lmstudio/qwen3.6-27b";

      # Todo tracking / verification / execution guidance.
      atlas.model = "lmstudio/qwen3.6-27b";

      # Main implementer / harness modifier.
      hephaestus = {
        model = "lmstudio/qwen3.6-27b";
        allow_non_gpt_model = true;
      };

      # Independent reasoning / architecture / assumption checking.
      oracle.model = "lmstudio/nvidia-nemotron-3-nano-omni-30b-a3b-reasoning";
      momus.model = "lmstudio/nvidia-nemotron-3-nano-omni-30b-a3b-reasoning";

      # Broad repo/IDA/docs exploration.
      explore.model = "lmstudio/qwen3.6-35b-a3b";
      librarian.model = "lmstudio/qwen3.6-35b-a3b";

      # Screenshots, diagrams, PDFs, UI traces, visual artifacts.
      multimodal-looker.model = "lmstudio/gemma-4-31b-it";

      # Junior helper, still code-capable enough for RE context.
      sisyphus-junior.model = "lmstudio/qwen3.6-35b-a3b";
    };

    categories = {
      # Visual inspection, UI/design, diagrams.
      visual-engineering.model = "lmstudio/gemma-4-31b-it";

      # Deep reverse-engineering/code reasoning.
      ultrabrain.model = "lmstudio/qwen3.6-27b";
      deep.model = "lmstudio/qwen3.6-27b";

      # Reports, naming, aesthetic/prose work.
      artistry.model = "lmstudio/gemma-4-31b-it";
      writing.model = "lmstudio/gemma-4-31b-it";

      # Fast/simple tasks using normal GLM, not REAP.
      quick.model = "lmstudio/glm-4.7-flash";

      # General defaults.
      unspecified-high.model = "lmstudio/qwen3.6-35b-a3b";
      unspecified-low.model = "lmstudio/glm-4.7-flash";
    };
  };
}
