from __future__ import annotations

import re
import time
from collections.abc import Iterable

from .config import Settings, get_settings

try:
    import serial
except ImportError:  # pragma: no cover - serial output is optional
    serial = None


FRAME_END = 0xFF
TEXT_CHUNK_WIDTH = 4
TEXT4_ALLOWED = re.compile(r"[^a-z0-9 !',.\-?#^]")


def serial_available() -> bool:
    return serial is not None


def _serial_session(settings: Settings):
    if serial is None:
        raise RuntimeError("pyserial is not installed")
    session = serial.Serial(settings.serial_port, settings.serial_baud, timeout=2)
    time.sleep(2.0)
    return session


def _normalize_text4(text: str) -> str:
    lower = text.lower().replace("\n", " ")
    ascii_text = TEXT4_ALLOWED.sub(" ", lower)
    compact = re.sub(r"\s+", " ", ascii_text).strip()
    return compact if compact else " "


def _chunk_string(text: str, width: int) -> Iterable[str]:
    for offset in range(0, len(text), width):
        yield text[offset:offset + width].ljust(width)


def _text_to_chunks(text: str, width: int = TEXT_CHUNK_WIDTH) -> list[str]:
    return list(_chunk_string(_normalize_text4(text), width))


def _write_lines(lines: Iterable[str], settings: Settings) -> None:
    with _serial_session(settings) as session:
        for line in lines:
            session.write(f"{line}\n".encode("ascii"))
            session.flush()
            if settings.serial_chunk_delay_ms > 0:
                time.sleep(settings.serial_chunk_delay_ms / 1000)


def _write_braille_frame(cells: bytes, settings: Settings) -> None:
    with _serial_session(settings) as session:
        session.write(bytes(cells) + bytes([FRAME_END]))
        session.flush()


def send_cells(cells: bytes, settings: Settings | None = None) -> None:
    _write_braille_frame(cells, settings or get_settings())


def send_text_chunks(text: str, settings: Settings | None = None) -> None:
    active = settings or get_settings()
    _write_lines(_text_to_chunks(text), active)


def _preview_hex(cells: bytes) -> str:
    return " ".join(format(value, "02x") for value in cells[:32])


def _disabled_reason(payload: dict) -> dict:
    payload["reason"] = "Device transport is disabled; preview mode only."
    return payload


def _braille_payload(cells: bytes, settings: Settings) -> dict:
    frame = bytes(cells) + bytes([FRAME_END])
    payload = {
        "transport": "serial",
        "format": "braille",
        "enabled": settings.enable_device_io,
        "sent": False,
        "cell_count": len(cells),
        "cell_bytes": list(cells),
        "frame_bytes": list(frame),
        "preview": _preview_hex(cells),
        "terminator": format(FRAME_END, "02x"),
    }
    if not settings.enable_device_io:
        return _disabled_reason(payload)
    try:
        send_cells(cells, settings=settings)
        payload["sent"] = True
    except Exception as exc:  # pragma: no cover - depends on attached hardware
        payload["error"] = str(exc)
    return payload


def _text4_payload(text: str, settings: Settings) -> dict:
    normalized = _normalize_text4(text)
    chunks = list(_chunk_string(normalized, TEXT_CHUNK_WIDTH))
    payload = {
        "transport": "serial",
        "format": "text4",
        "enabled": settings.enable_device_io,
        "sent": False,
        "normalized_text": normalized,
        "chunk_count": len(chunks),
        "chunks": chunks,
        "preview": " | ".join(chunks[:8]),
        "delimiter": "\\n",
    }
    if not settings.enable_device_io:
        return _disabled_reason(payload)
    try:
        _write_lines(chunks, settings)
        payload["sent"] = True
    except Exception as exc:  # pragma: no cover - depends on attached hardware
        payload["error"] = str(exc)
    return payload


def prepare_device_payload(text: str, cells: bytes, settings: Settings | None = None) -> dict:
    active = settings or get_settings()
    if active.device_format == "braille":
        return _braille_payload(cells, active)
    return _text4_payload(text, active)
