$ErrorActionPreference = "Stop"

if (-not (Test-Path ".venv")) {
  python -m venv .venv
}

. .\.venv\Scripts\Activate.ps1
pip install -r backend\requirements.txt
python -m uvicorn backend.main:app --host 127.0.0.1 --port 8000
