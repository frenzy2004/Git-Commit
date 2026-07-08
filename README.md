# Fingertips

**The visual and spoken world, delivered through touch.**

Fingertips is a classroom-access and assistive-communication prototype for deaf-blind learners and older adults with dual sensory impairment. It turns images and spoken audio into short, tactile-ready learning notes, then prepares the text for a four-character tactile hardware display.

Built for **Codex for Healthcare: From Prototype to Production** (A*STAR x IMDA x OpenAI), Singapore, July 2026.

**Track 5: Care for Older Adults** - safety, independence, and social support.

**Implementation credit:** this codebase was built end-to-end with **OpenAI Codex** as the primary coding agent, covering the browser UI, FastAPI backend, OpenAI model adapters, device payload pipeline, serial protocol, hardware firmware, tests, documentation, GitHub setup, and Vercel deployment workflow.

```text
Image or audio input -> essential learning note -> text4 chunks -> optional serial output
```

## The Problem

Singapore becomes a super-aged society in 2026, with 1 in 5 residents aged 65 or older. Inside that group is a population most digital health tools do not serve well: older adults with dual sensory impairment, meaning combined vision and hearing loss.

Most accessibility systems assume at least one working channel:

- Screen readers, apps, and dashboards assume a working eye.
- Voice assistants, alarms, and phone calls assume a working ear.
- For a deaf-blind person, touch may be the only reliable channel left.

Dual sensory loss can trigger isolation, social withdrawal, cognitive decline, and loss of independence. Fingertips restores a usable channel by turning visual scenes and spoken content into tactile output that can be read through the fingers.

## What It Does

Fingertips has two input modes, both ending in the same tactile output:

| Mode | Input | Pipeline | Output |
| --- | --- | --- | --- |
| Image | Photo, worksheet, slide, board, diagram, object, or scene | OpenAI vision model -> semantic compression -> device text | Short tactile-ready note naming what matters |
| Audio | Spoken audio from a lesson, conversation, or explanation | OpenAI transcription -> key-point summary -> device text | Short tactile-ready note carrying the essential idea |

The design principle is simple: **do not transcribe everything; keep the point.** A tactile reader cannot absorb a long wall of text efficiently, so the system compresses each input to the most important idea before it reaches the display.

The current hardware protocol emits four-character chunks for a tactile device flow. The same software boundary can support braille cells or other learnable tactile patterns as the hardware matures.

## System Architecture

```text
Browser UI  ->  FastAPI backend  ->  Text shaping  ->  Device payload  ->  Optional serial hardware

frontend/
  app.js
    - route-free SPA state
    - camera stream + canvas capture
    - file upload Blob handling
    - MediaRecorder audio capture
    - multipart API calls
    - result rendering and copy flow

backend/
  main.py
    GET  /health
    POST /image
    POST /lecture
    POST /realtime/session
  uploads.py
    MIME validation + bounded reads
  pipeline.py
    mode-specific orchestration + shared response contract

model adapters/
  vision.py
    image bytes -> OpenAI Responses API -> one tactile-ready note
  audio.py
    audio bytes -> OpenAI transcription -> transcript
  realtime.py
    browser SDP offer -> OpenAI Realtime session -> SDP answer
  summarizer.py
    transcript -> OpenAI Responses API -> key point

text + device/
  text_utils.py
    normalize, trim, and split learner-facing text
  serial_out.py
    text4 chunking, preview payload, optional serial write
  braille.py
    6-dot cell conversion path for braille-mode payloads

hardware/
  ESP32 firmware for motor-position display experiments
```

Design rule: the hardware stays simple. The intelligence lives in software so model, prompt, text-shaping, and device-protocol improvements can happen without redesigning the physical display.

## Technical Implementation

Fingertips is intentionally split into small runtime boundaries so each layer can be tested or replaced independently.

### Frontend Runtime

- Static HTML/CSS/JS with no build step required.
- `app.js` renders the full interface from a single root node and keeps UI state in a small in-memory state object.
- Camera input uses `navigator.mediaDevices.getUserMedia`, draws to a hidden `<canvas>`, then converts the frame to a JPEG `Blob`.
- Audio recording uses `MediaRecorder`, stores chunks, and sends a generated WebM `Blob`.
- Upload inputs preserve the browser `File` object and send it as multipart form data.
- The demo image path uses a committed static asset at `frontend/assets/demo-fig.png`.
- Result rendering consumes only the stable API contract: `mode`, `simple_text`, `simple_sentences`, `summary`, and `device`.

### Backend Runtime

- FastAPI app factory in `backend/main.py` keeps app creation testable.
- Upload policy is isolated in `uploads.py`, including MIME validation and maximum byte limits.
- `pipeline.py` is the orchestration boundary: it calls the selected input adapter, shapes text, builds device payloads, and returns one response shape for both endpoints.
- `config.py` reads `.env` once and normalizes booleans, integers, CSV origins, model names, device settings, serial settings, and upload limits.
- CORS is configurable through `CORS_ORIGINS` and defaults to permissive demo behavior.

### OpenAI Model Layer

- `vision.py` sends image data as a base64 data URL through the OpenAI Responses API.
- `audio.py` sends uploaded audio bytes to the OpenAI audio transcription endpoint.
- `realtime.py` brokers browser WebRTC sessions to OpenAI Realtime without exposing the server API key to frontend code.
- `summarizer.py` sends transcripts through the OpenAI Responses API with a tactile-reading prompt.
- Model IDs are environment-driven:
  - `OPENAI_MODEL`
  - `OPENAI_VISION_MODEL`
  - `OPENAI_SUMMARY_MODEL`
  - `OPENAI_TRANSCRIBE_MODEL`
  - `OPENAI_REALTIME_MODEL`
- Current defaults use `gpt-5.5` for image and summary reasoning, `gpt-4o-transcribe` for uploaded speech-to-text, and `gpt-realtime-2` for live WebRTC audio sessions.

### Text And Device Pipeline

- `essentialize_text()` removes noisy phrasing and keeps output short enough for tactile reading.
- `split_simple_sentences()` provides UI-friendly sentence cards while preserving the full summary.
- `prepare_device_payload()` normalizes text, chunks it, previews it, and optionally sends it to hardware.
- `DEVICE_FORMAT=text4` emits four-character chunks for the current tactile hardware flow.
- `DEVICE_FORMAT=braille` keeps the 6-dot cell conversion path available for braille-oriented hardware.
- Serial output is disabled by default and only writes when `ENABLE_DEVICE_IO=true`.

### Response Contract

All processing endpoints return the same core shape:

```json
{
  "mode": "image",
  "simple_text": "Short learner-facing note.",
  "simple_sentences": ["Short learner-facing note."],
  "summary": "Short learner-facing note.",
  "cell_count": 28,
  "braille_preview": "shor | t le",
  "device": {
    "transport": "serial",
    "format": "text4",
    "enabled": false,
    "sent": false,
    "normalized_text": "short learner-facing note.",
    "chunk_count": 7,
    "chunks": ["shor", "t le", "arne"]
  },
  "serial": {
    "transport": "serial",
    "format": "text4",
    "enabled": false
  }
}
```

### Test Surface

- API tests use FastAPI `TestClient` and environment monkeypatching.
- Device-text tests verify normalization, chunking, and payload behavior.
- Frontend syntax is checked with `node --check frontend/app.js`.
- CI runs through the repository test workflow on push.

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

Key backend modules:

- `main.py`: FastAPI app factory and routes.
- `config.py`: environment-backed settings.
- `uploads.py`: MIME and file-size policy.
- `pipeline.py`: image/audio processing flow.
- `vision.py`: OpenAI image understanding adapter.
- `audio.py`: OpenAI speech-to-text adapter.
- `summarizer.py`: transcript compression adapter.
- `text_utils.py`: sentence shaping helpers.
- `serial_out.py`: device payload and serial output.
- `braille.py`: braille cell conversion with a built-in ASCII mapping.

## Frontend

The frontend is a static browser app:

- Camera or file upload for images.
- Microphone recording or file upload for audio.
- A demo fig image for quick image-flow presentations.
- Result cards showing the essential summary and device-ready output.
- Clean demo behavior when network or API calls are unavailable.

Deployment:

[https://frontend-beta-amber-17.vercel.app](https://frontend-beta-amber-17.vercel.app)

## Backend

The backend exposes:

```text
GET  /health
POST /image
POST /lecture
```

Both processing endpoints return learner-facing summary text plus a device payload:

```json
{
  "mode": "image",
  "simple_text": "A whole fig and a cut fig show purple skin outside and many tiny seeds inside.",
  "simple_sentences": [
    "A whole fig and a cut fig show purple skin outside and many tiny seeds inside."
  ],
  "device": {
    "format": "text4",
    "chunks": ["a wh", "ole ", "fig "]
  }
}
```

See [docs/API.md](docs/API.md) for the full response shape.

## Device Flow

The backend emits newline-terminated four-character chunks:

```text
this
 pic
ture
```

The firmware reads each line, maps characters to calibrated motor positions, holds the display position, and releases the motors before the next chunk.

## Hardware Build

The prototype hardware is intentionally low-cost and repairable.

| Part | Qty | Role |
| --- | --- | --- |
| SG90 micro servo | 24 plus spares | One per tactile dot |
| PCA9685 16-channel PWM driver | 2 | Drives the servo array |
| ESP32 or Arduino-compatible controller | 1 | Receives serial frames and commands the drivers |
| 5V power supply, 5A or higher | 1 | Powers the servo rail |
| Guide plate and tactile pins | As needed | Creates the raised-dot surface |

The production path can replace the prototype servo array with purpose-built tactile or refreshable braille cells while keeping the backend pipeline and device protocol boundary intact.

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

PowerShell helpers are also available:

```powershell
scripts\start-backend.ps1
scripts\start-frontend.ps1
scripts\test.ps1
```

## Configuration

Copy `.env.example` to `backend/.env` and adjust values as needed.

Demo defaults:

```env
MOCK_OPENAI=true
MOCK_TRANSCRIBE=true
ENABLE_DEVICE_IO=false
DEVICE_FORMAT=text4
```

Live OpenAI mode:

```bash
pip install -r backend/requirements-full.txt
```

Then set:

```env
OPENAI_API_KEY=your_api_key_here
MOCK_OPENAI=false
MOCK_TRANSCRIBE=false
OPENAI_MODEL=gpt-5.5
OPENAI_VISION_MODEL=gpt-5.5
OPENAI_SUMMARY_MODEL=gpt-5.5
OPENAI_TRANSCRIBE_MODEL=gpt-4o-transcribe
OPENAI_REALTIME_MODEL=gpt-realtime-2
ENABLE_DEVICE_IO=true
SERIAL_PORT=your_serial_port_here
```

Important settings:

- `MOCK_OPENAI`: keeps image and summary flows demo-safe without external model calls.
- `MOCK_TRANSCRIBE`: keeps audio transcription demo-safe.
- `OPENAI_REALTIME_MODEL`: selects the live microphone Realtime model used by `/realtime/session`.
- `ENABLE_DEVICE_IO`: writes frames to the serial device when true; preview-only when false.
- `DEVICE_FORMAT`: `text4` for four-character chunks or `braille` for cell bytes.
- `SERIAL_PORT` and `SERIAL_BAUD`: device connection settings.

## Testing

```bash
pip install -r backend/requirements-dev.txt
pytest
node --check frontend/app.js
```

Current coverage includes:

- Health endpoint response.
- Image endpoint contract.
- Audio endpoint contract.
- Upload size limits.
- Text normalization.
- Four-character chunking.
- Summary trimming.

## Safety, Privacy, And Ethics

- **Privacy by design:** camera frames and recordings are processed for the current request and are not stored by the app.
- **User safety:** tactile pins should be low-force, rounded, and free of sharp edges or pinch points.
- **Built with users, not just for them:** a deaf-blind reader is the intended first user, not a demo prop.
- **Not a medical device:** Fingertips is an assistive communication prototype, not a diagnostic tool.

## From Prototype To Production

Today, Fingertips is a working prototype that turns scenes and speech into tactile-ready reading.

Next steps:

- Validate the interaction model with Singapore's visually impaired and deaf-blind communities.
- Improve the tactile hardware into a more robust refreshable display.
- Expand language, audio, and classroom workflows.
- Prepare the device and software pathway for clinical, eldercare, and assistive-technology pilots.

## AI Declaration

Built end-to-end with OpenAI Codex as the primary implementation agent. Codex generated and iterated the frontend, backend, firmware, serial protocol, OpenAI integration layer, tests, documentation, repository structure, deployment wiring, and commit history for the working prototype. Human direction, product decisions, testing judgment, and final acceptance were provided by the project owner.

Open-source components include FastAPI, pyserial, python-dotenv, OpenAI Python SDK, and optional liblouis braille translation.
