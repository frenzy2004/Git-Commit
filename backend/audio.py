from __future__ import annotations

from io import BytesIO

from .config import Settings, get_settings

try:
    from openai import OpenAI as OpenAIClient
except ImportError:  # pragma: no cover - live transcription is optional
    OpenAIClient = None


SAMPLE_TRANSCRIPT = (
    "This demo recording introduces a lesson, states the main point, "
    "and gives one short example. Disable MOCK_TRANSCRIBE to use OpenAI transcription."
)


def transcription_available(settings: Settings | None = None) -> bool:
    active = settings or get_settings()
    return active.mock_transcribe or OpenAIClient is not None


def _client(settings: Settings):
    if OpenAIClient is None:
        raise RuntimeError("openai is not installed")
    if settings.openai_api_key:
        return OpenAIClient(api_key=settings.openai_api_key)
    return OpenAIClient()


def warm_audio_model(settings: Settings | None = None) -> bool:
    return transcription_available(settings)


def _suffix_or_default(value: str) -> str:
    suffix = value.strip() or ".webm"
    return suffix if suffix.startswith(".") else f".{suffix}"


def transcribe(audio_bytes: bytes, suffix: str = ".webm", settings: Settings | None = None) -> str:
    active = settings or get_settings()
    if active.mock_transcribe:
        return SAMPLE_TRANSCRIPT

    audio_file = BytesIO(audio_bytes)
    audio_file.name = f"recording{_suffix_or_default(suffix)}"
    result = _client(active).audio.transcriptions.create(
        model=active.transcribe_model,
        file=audio_file,
    )
    return getattr(result, "text", str(result)).strip()
