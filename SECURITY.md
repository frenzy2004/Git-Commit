# Security

Fingertips is designed for local classroom demos and prototype hardware testing.

## Recommended Local Settings

- Keep the API bound to `127.0.0.1` unless you intentionally expose it.
- Set `CORS_ORIGINS` to trusted frontend origins for shared networks.
- Leave `ENABLE_DEVICE_IO=false` until the ESP32 is connected and supervised.
- Do not commit real `.env` files or API keys.

## Reporting Issues

Open a private issue or contact the repository owner if you find a way to:

- Trigger unwanted serial output.
- Bypass upload limits.
- Expose API keys, transcripts, or uploaded files.
- Cause unsafe hardware motion.
