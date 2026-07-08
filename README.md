# Fingertips

Fingertips is a classroom-access prototype for deaf-blind learners. It turns classroom images and spoken audio into short, tactile-ready learning notes, then prepares the text for a four-character ESP32 display.

```text
Image or audio input -> concise learning note -> text4 chunks -> optional serial output
```

## Highlights

- Static browser app for image capture, image upload, audio recording, and audio upload.
- FastAPI backend with stable demo-mode responses.
- Optional Claude adapters for image understanding and transcript summarization.
- Optional local transcription through `faster-whisper`.
- Four-character text chunking for tactile hardware playback.
- ESP32 firmware for motor-position display experiments.
- Tests, examples, helper scripts, and CI workflow included.

## Repository Map

```text
.github/workflows/   CI checks
backend/             FastAPI app, model adapters, text shaping, serial payloads
docs/                Architecture, API, device protocol, demo guide
examples/            Example API responses and text4 chunks
frontend/            Browser interface
hardware/            ESP32 firmware
scripts/             Local helper commands
tests/               API and text utility tests
```

## Documentation

- [Architecture](docs/ARCHITECTURE.md)
- [API](docs/API.md)
- [Device protocol](docs/DEVICE_PROTOCOL.md)
- [Demo guide](docs/DEMO_GUIDE.md)
- [Contributing](CONTRIBUTING.md)
- [Security](SECURITY.md)

## Quick Start

Install the default demo dependencies:

```bash
python -m venv .venv
.venv\Scripts\activate
pip install -r backend/requirements.txt
```

Start the backend:

```bash
python -m uvicorn backend.main:app --host 127.0.0.1 --port 8000
```

Start the frontend:

```bash
cd frontend
python -m http.server 4173 --bind 127.0.0.1
```

Open:

```text
http://127.0.0.1:4173
```

## Helper Scripts

PowerShell helpers are available for Windows:

```powershell
scripts\start-backend.ps1
scripts\start-frontend.ps1
scripts\test.ps1
```

## Configuration

Copy `.env.example` to `backend/.env` and adjust values as needed.

Demo defaults:

```env
MOCK_CLAUDE=true
MOCK_TRANSCRIBE=true
ENABLE_DEVICE_IO=false
DEVICE_FORMAT=text4
```

Live mode:

```bash
pip install -r backend/requirements-full.txt
```

Then set:

```env
ANTHROPIC_API_KEY=your_api_key_here
MOCK_CLAUDE=false
MOCK_TRANSCRIBE=false
ENABLE_DEVICE_IO=true
SERIAL_PORT=your_serial_port_here
```

## API Overview

```text
GET  /health
POST /image
POST /lecture
```

Both processing endpoints return learner-facing summary text plus a device payload:

```json
{
  "simple_text": "This picture has one important idea.",
  "simple_sentences": ["This picture has one important idea."],
  "device": {
    "format": "text4",
    "chunks": ["this", " pic", "ture"]
  }
}
```

See [docs/API.md](docs/API.md) for the full response shape.

## Device Flow

The backend emits newline-terminated four-character chunks:

```text
this\n
 pic\n
ture\n
```

The firmware reads each line, maps the characters to calibrated motor angles, holds the display position, and releases the motors before the next chunk.

## Testing

```bash
pip install -r backend/requirements-dev.txt
pytest
node --check frontend/app.js
```

Current test coverage includes:

- Health endpoint response.
- Image endpoint contract.
- Audio endpoint contract.
- Upload size limits.
- Text normalization.
- Four-character chunking.
- Summary trimming.

## Project Status

Fingertips is a prototype meant for demos, learning, and hardware iteration. Keep demo mode working by default, and enable live integrations only when API credentials, local transcription dependencies, and supervised hardware are ready.
