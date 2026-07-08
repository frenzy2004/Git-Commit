$ErrorActionPreference = "Stop"

Push-Location frontend
try {
  python -m http.server 4173 --bind 127.0.0.1
}
finally {
  Pop-Location
}
