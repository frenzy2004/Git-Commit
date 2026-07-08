# Fingertips

Fingertips is a classroom-access prototype for deaf-blind learners. It accepts classroom images or spoken audio, extracts the essential learning point, and prepares the result for a four-character tactile display driven by an ESP32.

The project is built for demos first: it runs without cloud keys by default, exposes clear API responses, and can switch to live model, transcription, and device I/O when the hardware and credentials are available.

## What Fingertips Does

- Captures or uploads textbook pages, slides, worksheets, diagrams, and scenes.
- Records or uploads classroom audio.
- Converts image or speech input into one or two short, plain-language sentences.
- Normalizes the final text into predictable ASCII for tactile playback.
- Splits output into four-character chunks for the ESP32 firmware.
- Provides serial preview data even when hardware output is disabled.
- Keeps the browser UI simple enough for a fast presentation or classroom workflow.

## Architecture

```text
frontend/
  index.html          Minimal browser shell
  app.js              UI rendering, camera/audio capture, API calls
  style.css           Responsive application styling

backend/
  main.py             FastAPI routes and app factory
  config.py           Environment-backed settings
  uploads.py          File type and size validation
  pipeline.py         Image/audio processing orchestration
  vision.py           Optional Claude image adapter
  summarizer.py       Optional Claude transcript adapter
  audio.py            Optional local Whisper transcription
  text_utils.py       Sentence splitting and summary trimming
  braille.py          Braille cell translation with fallback mapping
  serial_out.py       Text chunk and serial payload generation

hardware/
  Braille_hardware.ino  ESP32 firmware for four motor positions

tests/
  test_api.py
  test_device_text.py
```

## Quick Start

Use the standard dependency set for demo mode:

```bash
python -m venv .venv
.venv\Scripts\activate
pip install -r backend/requirements.txt
```

Start the API:

```bash
python -m uvicorn backend.main:app --host 127.0.0.1 --port 8000
```

Start the frontend in a second terminal:

```bash
cd frontend
python -m http.server 4173 --bind 127.0.0.1
```

Open:

```text
http://127.0.0.1:4173
```

Health check:

```text
http://127.0.0.1:8000/health
```

## Demo Mode

Demo mode is enabled by default:

```env
MOCK_CLAUDE=true
MOCK_TRANSCRIBE=true
ENABLE_DEVICE_IO=false
```

In this mode, image and audio endpoints return stable mock summaries. This keeps the app runnable on any laptop without API keys, model downloads, microphones, or attached hardware.

## Live Mode

Install the optional stack:

```bash
pip install -r backend/requirements-full.txt
```

Create `backend/.env`:

```env
ANTHROPIC_API_KEY=your_api_key_here
MOCK_CLAUDE=false
MOCK_TRANSCRIBE=false
ENABLE_DEVICE_IO=true
DEVICE_FORMAT=text4
SERIAL_PORT=your_serial_port_here
SERIAL_BAUD=115200
SERIAL_CHUNK_DELAY_MS=0
```

Optional tuning:

```env
ANTHROPIC_MODEL=claude-opus-4-8
ANTHROPIC_VISION_MODEL=claude-opus-4-8
ANTHROPIC_SUMMARY_MODEL=claude-opus-4-8
WHISPER_MODEL=base
CORS_ORIGINS=http://127.0.0.1:4173,http://localhost:4173
MAX_IMAGE_BYTES=10485760
MAX_AUDIO_BYTES=52428800
```

## API Contract

### `GET /health`

Returns service status, demo-mode state, device transport settings, and dependency availability.

### `POST /image`

Accepts an image file as multipart field `file`.

Supported types:

```text
image/jpeg
image/png
image/webp
image/gif
```

### `POST /lecture`

Accepts an audio file as multipart field `file`.

Supported common inputs include WebM, WAV, MP3, OGG, MP4, M4A, and AAC.

### Response Shape

Both processing endpoints return:

```json
{
  "mode": "image",
  "simple_text": "Short learner-facing text.",
  "simple_sentences": ["Short learner-facing text."],
  "summary": "Short learner-facing text.",
  "cell_count": 12,
  "braille_preview": "hex preview or text chunk preview",
  "device": {
    "transport": "serial",
    "format": "text4",
    "enabled": false,
    "sent": false,
    "chunks": ["shor", "t le"]
  },
  "serial": {}
}
```

`serial` is kept as an alias for older UI integrations. New code should prefer `device`.
