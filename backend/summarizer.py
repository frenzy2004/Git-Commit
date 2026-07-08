from __future__ import annotations

from .config import Settings, get_settings

try:
    from openai import OpenAI as OpenAIClient
except ImportError:  # pragma: no cover - live model calls are optional
    OpenAIClient = None


PROMPT_PARTS = {
    "role": (
        "You are preparing classroom speech for a learner who reads through touch. "
        "The output must be a short learning note, not a transcript."
    ),
    "limits": (
        "Plain ASCII only. No markdown. No bullets. No emoji. "
        "Skip greetings, filler words, names, repeated phrases, and examples unless an example is the whole lesson."
    ),
    "shape": (
        "Use simple vocabulary. Prefer one compact sentence. Use two only when the transcript has two essential ideas."
    ),
}

TRANSCRIPT_PREFIX = "Find the key point in this classroom transcript and rewrite it for tactile reading:\n\n"
PLACEHOLDER_SUMMARY = "This demo recording has one main point. Turn off MOCK_OPENAI to summarize real audio."


def summary_available(settings: Settings | None = None) -> bool:
    active = settings or get_settings()
    return active.mock_openai or OpenAIClient is not None


def _system_text() -> str:
    return " ".join(PROMPT_PARTS.values())


def _client(settings: Settings):
    if OpenAIClient is None:
        raise RuntimeError("openai is not installed")
    if settings.openai_api_key:
        return OpenAIClient(api_key=settings.openai_api_key)
    return OpenAIClient()


def summarize_transcript(transcript: str, settings: Settings | None = None) -> str:
    active = settings or get_settings()
    if active.mock_openai:
        return PLACEHOLDER_SUMMARY

    response = _client(active).responses.create(
        model=active.summary_model,
        instructions=_system_text(),
        input=TRANSCRIPT_PREFIX + transcript,
        max_output_tokens=512,
        text={"verbosity": "low"},
    )
    return response.output_text.strip()
