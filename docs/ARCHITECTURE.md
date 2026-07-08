# Architecture

Fingertips has three runtime surfaces:

```text
Browser UI  ->  FastAPI backend  ->  Optional ESP32 serial device
```

## Frontend

The frontend is a static browser app:

- `frontend/index.html` provides the document shell.
- `frontend/app.js` renders the interface and owns camera/audio state.
- `frontend/style.css` handles responsive desktop and mobile layout.

The app calls the backend with multipart uploads and renders the returned `simple_text`, `simple_sentences`, and `device` preview data.

## Backend

The backend keeps each boundary in a separate module:

- `main.py`: FastAPI app factory and routes.
- `config.py`: environment-backed settings.
- `uploads.py`: MIME and file-size policy.
- `pipeline.py`: image/audio processing flow.
- `vision.py`: image model adapter.
- `audio.py`: local speech-to-text adapter.
- `summarizer.py`: transcript compression adapter.
- `text_utils.py`: sentence shaping helpers.
- `serial_out.py`: device payload and serial output.
- `braille.py`: braille cell conversion with fallback mapping.

## Device

The firmware reads newline-delimited four-character chunks over serial. Each character maps to a calibrated motor angle. The backend defaults to `DEVICE_FORMAT=text4` so the API and firmware speak the same protocol.
