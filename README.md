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
