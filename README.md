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
