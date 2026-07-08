from __future__ import annotations

from dataclasses import dataclass
import os
from pathlib import Path

from dotenv import load_dotenv


load_dotenv(Path(__file__).with_name(".env"))


def parse_bool(value: str | None, default: bool) -> bool:
    if value is None:
        return default
    return value.strip().lower() in {"1", "true", "yes", "on"}


def parse_int(value: str | None, default: int) -> int:
    if value is None or value.strip() == "":
        return default
    try:
        return int(value)
    except ValueError:
        return default


def parse_csv(value: str | None, default: tuple[str, ...]) -> tuple[str, ...]:
    if not value:
        return default
    items = tuple(item.strip() for item in value.split(",") if item.strip())
    return items or default


@dataclass(frozen=True)
class Settings:
    mock_openai: bool
    mock_transcribe: bool
    enable_device_io: bool
    device_format: str
    serial_port: str
    serial_baud: int
    serial_chunk_delay_ms: int
    openai_api_key: str | None
    vision_model: str
    summary_model: str
    transcribe_model: str
    realtime_model: str
    cors_origins: tuple[str, ...]
    max_image_bytes: int
    max_audio_bytes: int


def get_settings() -> Settings:
    device_format = os.getenv("DEVICE_FORMAT", "text4").strip().lower()
    if device_format not in {"text4", "braille"}:
        device_format = "text4"

    default_baud = 115200 if device_format == "text4" else 9600
    default_model = os.getenv("OPENAI_MODEL", "gpt-5.5")

    return Settings(
        mock_openai=parse_bool(os.getenv("MOCK_OPENAI"), True),
        mock_transcribe=parse_bool(os.getenv("MOCK_TRANSCRIBE"), True),
        enable_device_io=parse_bool(os.getenv("ENABLE_DEVICE_IO"), False),
        device_format=device_format,
        serial_port=os.getenv("SERIAL_PORT", "/dev/cu.usbmodem1101"),
        serial_baud=parse_int(os.getenv("SERIAL_BAUD"), default_baud),
        serial_chunk_delay_ms=parse_int(os.getenv("SERIAL_CHUNK_DELAY_MS"), 0),
        openai_api_key=os.getenv("OPENAI_API_KEY"),
        vision_model=os.getenv("OPENAI_VISION_MODEL", default_model),
        summary_model=os.getenv("OPENAI_SUMMARY_MODEL", default_model),
        transcribe_model=os.getenv("OPENAI_TRANSCRIBE_MODEL", "gpt-4o-transcribe"),
        realtime_model=os.getenv("OPENAI_REALTIME_MODEL", "gpt-realtime-2"),
        cors_origins=parse_csv(os.getenv("CORS_ORIGINS"), ("*",)),
        max_image_bytes=parse_int(os.getenv("MAX_IMAGE_BYTES"), 10 * 1024 * 1024),
        max_audio_bytes=parse_int(os.getenv("MAX_AUDIO_BYTES"), 50 * 1024 * 1024),
    )
