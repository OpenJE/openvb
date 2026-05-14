# ./devenv/opencode/plugin/oh-my-openagent.nix

{ lib, config, ... }:

let
  cfg = config.ohMyOpenagent;

  schema = "https://raw.githubusercontent.com/code-yeongyu/oh-my-openagent/v4.0.0/assets/oh-my-opencode.schema.json";

  localAgents = {
    sisyphus.model =          "lmstudio/qwen3.6-27b";
    atlas.model =             "lmstudio/qwen3.6-27b";
    sisyphus-junior.model =   "lmstudio/qwen3.6-35b-a3b";
    multimodal-looker.model = "lmstudio/gemma-4-31b-it";
    prometheus.model =        "lmstudio/qwen3.6-27b";
    metis.model =             "lmstudio/qwen3.6-27b";
    oracle.model =            "lmstudio/nvidia-nemotron-3-nano-omni-30b-a3b-reasoning";
    momus.model =             "lmstudio/nvidia-nemotron-3-nano-omni-30b-a3b-reasoning";
    librarian.model =         "lmstudio/qwen3.6-35b-a3b";
    explore.model =           "lmstudio/qwen3.6-35b-a3b";
    hephaestus.model =        "lmstudio/qwen3.6-27b";

    hephaestus.allow_non_gpt_model = true;
  };

  providerAgents = {
    sisyphus.model =          "opencode-go/kimi-k2.6";
    atlas.model =             "opencode-go/kimi-k2.6";
    sisyphus-junior.model =   "opencode-go/kimi-k2.6";
    multimodal-looker.model = "opencode-go/kimi-k2.6";
    prometheus.model =        "opencode-go/glm-5.1";
    metis.model =             "opencode-go/glm-5.1";
    oracle.model =            "opencode-go/glm-5.1";
    momus.model =             "opencode-go/glm-5.1";
    librarian.model =         "opencode-go/qwen3.5-plus";
    explore.model =           "opencode-go/qwen3.5-plus";
    hephaestus.model =        "opencode/gpt-5.5";
  };

  localCategories = {
    visual-engineering.model = "lmstudio/gemma-4-31b-it";
    ultrabrain.model =         "lmstudio/qwen3.6-27b";
    deep.model =               "lmstudio/qwen3.6-27b";
    artistry.model =           "lmstudio/gemma-4-31b-it";
    quick.model =              "lmstudio/glm-4.7-flash";
    unspecified-high.model =   "lmstudio/qwen3.6-35b-a3b";
    unspecified-low.model =    "lmstudio/glm-4.7-flash";
    writing.model =            "lmstudio/gemma-4-31b-it";
  };

  localReCategories = {
    re-discovery.model = "lmstudio/qwen3.6-35b-a3b";
    re-analysis.model =  "lmstudio/nvidia-nemotron-3-nano-omni-30b-a3b-reasoning";
    re-review.model =    "lmstudio/nvidia-nemotron-3-nano-omni-30b-a3b-reasoning";
    re-synthesis.model = "lmstudio/qwen3.6-27b";
    re-ops.model =       "lmstudio/glm-4.7-flash";
  };

  providerCategories = {
    visual-engineering.model = "opencode-go/glm-5.1";
    ultrabrain.model =         "opencode-go/glm-5.1";
    deep.model =               "opencode/gpt-5.5";
    artistry.model =           "opencode/gemini-3.1-pro";
    quick.model =              "opencode-go/minimax-m2.7";
    unspecified-high.model =   "opencode-go/glm-5.1";
    unspecified-low.model =    "opencode-go/kimi-k2.6";
    writing.model =            "opencode-go/kimi-k2.6";
  };

  providerReCategories = {
    re-discovery.model = "opencode-go/qwen3.5-plus";
    re-analysis.model =  "opencode-go/glm-5.1";
    re-review.model =    "opencode-go/glm-5.1";
    re-synthesis.model = "opencode-go/kimi-k2.6";
    re-ops.model =       "opencode-go/minimax-m2.7";
  };
in
{
  options.ohMyOpenagent = {
    useLocalModels = lib.mkOption {
      type = lib.types.bool;
      default = true;
      description = ''
        Whether oh-my-openagent should use local lmstudio models instead of
        opencode provider models.
      '';
    };

    useReCloudModels = lib.mkOption {
      type = lib.types.bool;
      default = cfg.useLocalModels;
      description = ''
        Whether reverse-engineering categories should use opencode provider
        models instead of local lmstudio models.
      '';
    };
  };

  config = {
    files.".opencode/oh-my-openagent.jsonc".text = builtins.toJSON
      {
        "$schema" = schema;

        agents =
          if cfg.useLocalModels
          then localAgents
          else providerAgents;

        categories =
          (if cfg.useLocalModels
           then localCategories
           else providerCategories)
          // (if cfg.useReCloudModels
              then providerReCategories
              else localReCategories);

        team_mode = {
          enabled = true;
          max_parallel_members = 4;
          max_members = 8;
          tmux_visualization = false;
        };
      };
  };
}
