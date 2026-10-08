# NatLang Studio Easy: run an already bundled llama-server, or one available in PATH.
# Local model is preferred; if missing, llama.cpp downloads GGUF from Hugging Face on first launch.
param(
  [ValidateSet('qwen3','smollm2')][string]$Model = 'qwen3',
  [int]$Port = 8080,
  [string]$GGUFPath = ''
)
$ErrorActionPreference = 'Stop'
if ($Port -lt 1024 -or $Port -gt 65535) { throw 'Port must be 1024..65535' }
$root = Split-Path -Parent $PSScriptRoot
$localServer = Join-Path $root 'ai\bin\llama-server.exe'
if (Test-Path -LiteralPath $localServer) { $server = (Resolve-Path -LiteralPath $localServer).Path }
else {
  $found = Get-Command llama-server -ErrorAction SilentlyContinue
  if (-not $found) { throw 'llama-server non trovato. Scarica il pacchetto NatLang Studio Easy completo oppure installa llama.cpp.' }
  $server = $found.Source
}
$modelFile = if ($GGUFPath) { $GGUFPath } elseif ($Model -eq 'qwen3') { Join-Path $root 'ai\models\Qwen3-0.6B-Q4_K_M.gguf' } else { '' }
if ($GGUFPath -and -not (Test-Path -LiteralPath $GGUFPath -PathType Leaf)) { throw "GGUF non trovato: $GGUFPath" }
Write-Host "NatLang local AI at http://127.0.0.1:$Port (CTRL+C per interrompere)"
if ($modelFile -and (Test-Path -LiteralPath $modelFile -PathType Leaf)) {
  & $server -m $modelFile --alias local-model --host 127.0.0.1 --port $Port -c 4096
} else {
  $modelId = if ($Model -eq 'qwen3') { 'tensorblock/Qwen_Qwen3-0.6B-GGUF:Q4_K_M' } else { 'tensorblock/SmolLM2-360M-Instruct-GGUF:Q4_K_M' }
  Write-Host "Modello non incluso: scaricamento al primo avvio da Hugging Face ($modelId)"
  & $server -hf $modelId --alias local-model --host 127.0.0.1 --port $Port -c 4096
}
if ($LASTEXITCODE -ne 0) { throw "llama-server failed with exit code $LASTEXITCODE" }
