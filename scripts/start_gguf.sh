#!/usr/bin/env bash
set -euo pipefail
MODEL="${1:-qwen3}"
PORT="${2:-8080}"
if ! command -v llama-server >/dev/null 2>&1; then
  echo 'Please install llama.cpp llama-server and add it to PATH.' >&2; exit 1
fi
if ! [[ "$PORT" =~ ^[0-9]+$ ]] || (( PORT < 1024 || PORT > 65535 )); then
  echo 'Port must be between 1024 and 65535' >&2; exit 2
fi
case "$MODEL" in
  qwen3) MODEL_ID='tensorblock/Qwen_Qwen3-0.6B-GGUF:Q4_K_M';;
  smollm2) MODEL_ID='tensorblock/SmolLM2-360M-Instruct-GGUF:Q4_K_M';;
  *.gguf) exec llama-server -m "$MODEL" --alias local-model --host 127.0.0.1 --port "$PORT" -c 4096;;
  *) echo 'Select qwen3, smollm2, or a local .gguf file' >&2; exit 2;;
esac
exec llama-server -hf "$MODEL_ID" --alias local-model --host 127.0.0.1 --port "$PORT" -c 4096
