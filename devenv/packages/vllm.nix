# ./modules/packages/mcp-nixos.nix

{ inputs, ... }:
let
  vllm = inputs.nixpkgs-master.python313.withPackages (ps: [
    ps.vllm
    ps.huggingface-hub
    ps.openai
  ]);

  hfFile = repo: revision: file: hash:
    inputs.nixpkgs-master.fetchurl {
      url = "https://huggingface.co/${repo}/resolve/${revision}/${file}";
      inherit hash;
    };

  mkHfModel = name: repo: revision: files:
    inputs.nixpkgs-master.runCommand name { } ''
      mkdir -p "$out"

      ${builtins.concatStringsSep "\n" (
        map
          (f: ''
            mkdir -p "$out/$(dirname "${f.path}")"
            ln -s ${hfFile repo revision f.path f.hash} "$out/${f.path}"
          '')
          files
      )}
    '';

  qwen25_05b = mkHfModel
    "qwen2.5-0.5b-instruct-vllm"
    "Qwen/Qwen2.5-0.5B-Instruct"
    "main"
    [
      {
        path = "config.json";
        hash = "sha256-AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA=";
      }
      {
        path = "generation_config.json";
        hash = "sha256-AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA=";
      }
      {
        path = "tokenizer.json";
        hash = "sha256-AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA=";
      }
      {
        path = "tokenizer_config.json";
        hash = "sha256-AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA=";
      }
      {
        path = "model.safetensors";
        hash = "sha256-AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA=";
      }
    ];
in {
  packages = [
    vllm
  ];

  env = {
    VLLM_MODEL_QWEN25_05B = "${qwen25_05b}";
  };

  scripts = {
    check-vllm.exec = ''
      echo "vLLM:"
      python - <<'PY'
      import vllm
      print(vllm.__version__)
      PY

      echo
      echo "vLLM model dir:"
      echo "$VLLM_MODEL_QWEN25_05B"
      ls -lah "$VLLM_MODEL_QWEN25_05B"
    '';

    serve-vllm.exec = ''
      vllm serve "$VLLM_MODEL_QWEN25_05B" \
        --served-model-name qwen25-05b \
        --host 0.0.0.0 \
        --port 8000 \
        --gpu-memory-utilization 0.55 \
        --max-model-len 8192
    '';
  };
}
