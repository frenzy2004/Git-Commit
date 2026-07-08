from __future__ import annotations

import importlib
import importlib.util
from pathlib import Path
from tempfile import TemporaryDirectory

from .config import Settings, get_settings


SAMPLE_TRANSCRIPT = (
    "This demo recording introduces a lesson, states the main point, "
    "and gives one short example. Disable MOCK_TRANSCRIBE to run Whisper."
)

_MODEL_CACHE: dict[str, object] = {}


def _whisper_installed() -> bool:
    return importlib.util.find_spec("faster_whisper") is not None


def transcription_available(settings: Settings | None = None) -> bool:
    active = settings or get_settings()
    return active.mock_transcribe or _whisper_installed()


def _whisper_model_class():
    if not _whisper_installed():
        raise RuntimeError("faster-whisper is not installed")
    return importlib.import_module("faster_whisper").WhisperModel


def _model_for(settings: Settings):
    model_key = settings.whisper_model
    if model_key not in _MODEL_CACHE:
        model_type = _whisper_model_class()
        _MODEL_CACHE[model_key] = model_type(model_key, device="cpu", compute_type="int8")
    return _MODEL_CACHE[model_key]


def warm_audio_model(settings: Settings | None = None) -> bool:
    _model_for(settings or get_settings())
    return True


def _suffix_or_default(value: str) -> str:
    suffix = value.strip() or ".webm"
    return suffix if suffix.startswith(".") else f".{suffix}"


def _segment_text(segments) -> str:
    transcript_parts: list[str] = []
    for segment in segments:
        text = getattr(segment, "text", "").strip()
        if text:
            transcript_parts.append(text)
    return " ".join(transcript_parts).strip()


def transcribe(audio_bytes: bytes, suffix: str = ".webm", settings: Settings | None = None) -> str:
    active = settings or get_settings()
    if active.mock_transcribe:
        return SAMPLE_TRANSCRIPT

    with TemporaryDirectory(prefix="fingertips-audio-") as temp_dir:
        upload_path = Path(temp_dir, f"recording{_suffix_or_default(suffix)}")
        upload_path.write_bytes(audio_bytes)
        segments, _metadata = _model_for(active).transcribe(str(upload_path), beam_size=5)
        return _segment_text(segments)
