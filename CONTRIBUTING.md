# Contributing

Fingertips is organized so UI work, API work, and device work can move independently.

## Local Checks

Run these before opening a pull request:

```bash
pip install -r backend/requirements-dev.txt
pytest
node --check frontend/app.js
```

## Code Style

- Keep the browser app dependency-free unless a feature clearly needs a package.
- Keep backend modules scoped by boundary: routing, configuration, uploads, model adapters, text shaping, and device transport.
- Keep hardware protocol changes documented in `docs/DEVICE_PROTOCOL.md`.
- Keep demo mode working without API keys or attached hardware.

## Pull Request Notes

Include:

- What changed.
- How you tested it.
- Any environment variables or hardware assumptions.
