# API

Default base URL:

```text
http://127.0.0.1:8000
```

## `GET /health`

Returns demo state, service availability, and device transport settings.

Example:

```json
{
  "status": "ok",
  "demo_mode": true,
  "device_transport": {
    "enabled": false,
    "mode": "preview-only",
    "format": "text4"
  }
}
```

## `POST /image`

Multipart field:

```text
file
```

Supported media:

```text
image/jpeg
image/png
image/webp
image/gif
```

## `POST /lecture`

Multipart field:

```text
file
```

Supported common audio inputs:

```text
webm, wav, mp3, ogg, mp4, m4a, aac
```

## Processing Response

```json
{
  "mode": "image",
  "simple_text": "The image shows one important idea.",
  "simple_sentences": ["The image shows one important idea."],
  "summary": "The image shows one important idea.",
  "cell_count": 34,
  "braille_preview": "the  | imag | e sh",
  "device": {
    "transport": "serial",
    "format": "text4",
    "enabled": false,
    "sent": false,
    "normalized_text": "the image shows one important idea.",
    "chunk_count": 9,
    "chunks": ["the ", "imag", "e sh"]
  },
  "serial": {}
}
```
