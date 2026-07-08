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
    mock_claude: bool
    mock_transcribe: bool
    enable_device_io: bool
    device_format: str
    serial_port: str
    serial_baud: int
    serial_chunk_delay_ms: int
    anthropic_api_key: str | None
    vision_model: str
    summary_model: str
    whisper_model: str
    cors_origins: tuple[str, ...]
    max_image_bytes: int
    max_audio_bytes: int


def get_settings() -> Settings:
    device_format = os.getenv("DEVICE_FORMAT", "text4").strip().lower()
    if device_format not in {"text4", "braille"}:
        device_format = "text4"

    default_baud = 115200 if device_format == "text4" else 9600
    default_model = os.getenv("ANTHROPIC_MODEL", "claude-opus-4-8")

    return Settings(
        mock_claude=parse_bool(os.getenv("MOCK_CLAUDE"), True),
        mock_transcribe=parse_bool(os.getenv("MOCK_TRANSCRIBE"), True),
        enable_device_io=parse_bool(os.getenv("ENABLE_DEVICE_IO"), False),
        device_format=device_format,
        serial_port=os.getenv("SERIAL_PORT", "/dev/cu.usbmodem1101"),
        serial_baud=parse_int(os.getenv("SERIAL_BAUD"), default_baud),
        serial_chunk_delay_ms=parse_int(os.getenv("SERIAL_CHUNK_DELAY_MS"), 0),
        anthropic_api_key=os.getenv("ANTHROPIC_API_KEY"),
        vision_model=os.getenv("ANTHROPIC_VISION_MODEL", default_model),
        summary_model=os.getenv("ANTHROPIC_SUMMARY_MODEL", default_model),
        whisper_model=os.getenv("WHISPER_MODEL", "base"),
        cors_origins=parse_csv(os.getenv("CORS_ORIGINS"), ("*",)),
        max_image_bytes=parse_int(os.getenv("MAX_IMAGE_BYTES"), 10 * 1024 * 1024),
        max_audio_bytes=parse_int(os.getenv("MAX_AUDIO_BYTES"), 50 * 1024 * 1024),
    )
