# Fingertips

**The visual and spoken world, delivered through touch.**

Fingertips is a classroom-access and assistive-communication prototype for deaf-blind learners and older adults with dual sensory impairment. It turns images and spoken audio into short, tactile-ready learning notes, then prepares the text for a four-character tactile hardware display.

Built for **Codex for Healthcare: From Prototype to Production** (A*STAR x IMDA x OpenAI), Singapore, July 2026.

**Track 5: Care for Older Adults** - safety, independence, and social support.

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
Browser UI  ->  FastAPI backend  ->  Optional ESP32 serial device

frontend/
  camera, image upload, mic recording, audio upload

backend/
  /health
  /image
  /lecture

pipeline/
  image/audio input
  -> OpenAI model adapters
  -> essential text shaping
  -> text4 device payload
  -> optional serial transport

hardware/
  ESP32 firmware for motor-position display experiments
```

Design rule: the hardware stays simple. The intelligence lives in software so model, prompt, and device-protocol improvements can happen without redesigning the physical display.

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
ENABLE_DEVICE_IO=true
SERIAL_PORT=your_serial_port_here
```

Important settings:

- `MOCK_OPENAI`: keeps image and summary flows demo-safe without external model calls.
- `MOCK_TRANSCRIBE`: keeps audio transcription demo-safe.
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

Built with OpenAI Codex assistance for frontend, backend, firmware, serial protocol, and pipeline implementation. Live image understanding, summarization, and speech-to-text use OpenAI model adapters when configured.

Open-source components include FastAPI, pyserial, python-dotenv, OpenAI Python SDK, and optional liblouis braille translation.
