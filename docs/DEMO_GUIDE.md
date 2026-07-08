# Demo Guide

Use this flow for a quick presentation.

## 1. Start Services

```bash
scripts\start-backend.ps1
scripts\start-frontend.ps1
```

Open:

```text
http://127.0.0.1:4173
```

## 2. Show Image Flow

1. Open the Image screen.
2. Upload or capture an image.
3. Select `Summarize`.
4. Point out the short text and device preview behavior.

## 3. Show Audio Flow

1. Open the Audio screen.
2. Record or upload audio.
3. Select `Summarize`.
4. Explain that demo mode can run without a model download.

## 4. Hardware Talking Points

- The backend produces four-character chunks.
- The firmware reads one chunk per serial line.
- Each character maps to a motor angle.
- Hardware output stays disabled until `ENABLE_DEVICE_IO=true`.
