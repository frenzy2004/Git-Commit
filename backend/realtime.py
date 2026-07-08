from __future__ import annotations

import json
import secrets
import urllib.error
import urllib.request

from .config import Settings, get_settings


REALTIME_CALLS_URL = "https://api.openai.com/v1/realtime/calls"
REALTIME_INSTRUCTIONS = (
    "You are Fingertip, an assistive audio interface for deaf, blind, and deafblind users. "
    "Listen to the user's speech and return only one compact sentence with the essential idea. "
    "Plain ASCII only. No markdown, no bullet points, no filler, no greeting."
)


def realtime_available(settings: Settings | None = None) -> bool:
    active = settings or get_settings()
    return bool(active.openai_api_key and active.realtime_model)


def realtime_session_config(settings: Settings) -> dict:
    return {
        "type": "realtime",
        "model": settings.realtime_model,
        "output_modalities": ["text"],
        "instructions": REALTIME_INSTRUCTIONS,
    }


def _multipart_body(fields: dict[str, str]) -> tuple[bytes, str]:
    boundary = f"fingertip-{secrets.token_hex(16)}"
    chunks: list[bytes] = []
    for name, value in fields.items():
        chunks.append(f"--{boundary}\r\n".encode("ascii"))
        chunks.append(f'Content-Disposition: form-data; name="{name}"\r\n\r\n'.encode("ascii"))
        chunks.append(value.encode("utf-8"))
        chunks.append(b"\r\n")
    chunks.append(f"--{boundary}--\r\n".encode("ascii"))
    return b"".join(chunks), boundary


def create_realtime_call(offer_sdp: str, settings: Settings | None = None) -> str:
    active = settings or get_settings()
    if not active.openai_api_key:
        raise RuntimeError("OpenAI API key is not configured")
    if not active.realtime_model:
        raise RuntimeError("OpenAI realtime model is not configured")

    body, boundary = _multipart_body(
        {
            "sdp": offer_sdp,
            "session": json.dumps(realtime_session_config(active), separators=(",", ":")),
        }
    )
    request = urllib.request.Request(
        REALTIME_CALLS_URL,
        data=body,
        headers={
            "Authorization": f"Bearer {active.openai_api_key}",
            "Content-Type": f"multipart/form-data; boundary={boundary}",
            "OpenAI-Safety-Identifier": "fingertip-demo-user",
        },
        method="POST",
    )

    try:
        with urllib.request.urlopen(request, timeout=30) as response:
            return response.read().decode("utf-8")
    except urllib.error.HTTPError as exc:
        detail = exc.read().decode("utf-8", errors="replace").strip()
        raise RuntimeError(detail or f"OpenAI returned HTTP {exc.code}") from exc
    except urllib.error.URLError as exc:
        raise RuntimeError(str(exc.reason)) from exc
