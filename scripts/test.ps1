$ErrorActionPreference = "Stop"

if (-not (Test-Path ".venv")) {
  python -m venv .venv
}

. .\.venv\Scripts\Activate.ps1
pip install -r backend\requirements-dev.txt
pytest
node --check frontend\app.js
