# ./modules/packages/mcp-nixos.nix

{ inputs, ... }:
let
  llamaCppCuda = inputs.nixpkgs-master.llama-cpp.override {
    cudaSupport = true;
  };
  qwen25_7b_q4 = inputs.nixpkgs-master.fetchurl {
    url = "https://huggingface.co/bartowski/Qwen2.5-7B-Instruct-GGUF/resolve/main/Qwen2.5-7B-Instruct-Q4_K_M.gguf";
    hash = "sha256-AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA=";
  };
in {
  packages = [
    llamaCppCuda
  ];

  env = {
    LLAMA_MODEL_QWEN25_7B_Q4 = "${qwen25_7b_q4}";
  };

  scripts = {
    check-llama.exec = ''
      echo "llama.cpp:"
      llama-server --version || true

      echo
      echo "llama model:"
      echo "$LLAMA_MODEL_QWEN25_7B_Q4"
      test -f "$LLAMA_MODEL_QWEN25_7B_Q4"
    '';

    serve-llama.exec = ''
      llama-server \
        -m "$LLAMA_MODEL_QWEN25_7B_Q4" \
        --host 0.0.0.0 \
        --port 8080 \
        -ngl 999 \
        -c 8192
    '';
  };
}
