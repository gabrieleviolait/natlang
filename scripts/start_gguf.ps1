# Run from a terminal with llama-server installed (winget install llama.cpp).
# Downloads quantized weights via llama.cpp on first invocation; no weights are bundled.
param(
    [ValidateSet('qwen3','smollm2')][string]$Model = 'qwen3',
    [int]$Port = 8080,
    [string]$GGUFPath = ''
)
$ErrorActionPreference = 'Stop'
if ($Port -lt 1024 -or $Port -gt 65535) { throw 'Port must be between 1024 and 65535.' }
$exe = Get-Command llama-server -ErrorAction SilentlyContinue
if (-not $exe) { throw 'Install llama.cpp and ensure llama-server is in PATH (winget install llama.cpp).' }
if ($GGUFPath) {
    if (-not (Test-Path -LiteralPath $GGUFPath -PathType Leaf)) { throw "GGUF file not found: $GGUFPath" }
    & $exe.Source -m $GGUFPath --alias local-model --host 127.0.0.1 --port $Port -c 4096
} else {
    $repo = if ($Model -eq 'qwen3') { 'tensorblock/Qwen_Qwen3-0.6B-GGUF:Q4_K_M' } else { 'tensorblock/SmolLM2-360M-Instruct-GGUF:Q4_K_M' }
    Write-Host "Launching local model: $repo (first launch may download weights)"
    & $exe.Source -hf $repo --alias local-model --host 127.0.0.1 --port $Port -c 4096
}
