from __future__ import annotations

from .config import Settings, get_settings

try:
    from anthropic import Anthropic as AnthropicClient
except ImportError:  # pragma: no cover - cloud summarization is optional
    AnthropicClient = None


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
PLACEHOLDER_SUMMARY = "This lesson has one main point. Disable MOCK_CLAUDE to summarize real audio."


def summary_available(settings: Settings | None = None) -> bool:
    active = settings or get_settings()
    return active.mock_claude or AnthropicClient is not None


def _system_text() -> str:
    return " ".join(PROMPT_PARTS.values())


def _anthropic(settings: Settings):
    if AnthropicClient is None:
        raise RuntimeError("anthropic is not installed")
    if settings.anthropic_api_key:
        return AnthropicClient(api_key=settings.anthropic_api_key)
    return AnthropicClient()


def _message_text(message) -> str:
    parts: list[str] = []
    for item in getattr(message, "content", []):
        if getattr(item, "type", None) == "text":
            parts.append(item.text)
    return "".join(parts).strip()


def summarize_transcript(transcript: str, settings: Settings | None = None) -> str:
    active = settings or get_settings()
    if active.mock_claude:
        return PLACEHOLDER_SUMMARY

    response = _anthropic(active).messages.create(
        model=active.summary_model,
        max_tokens=512,
        system=_system_text(),
        messages=[
            {
                "role": "user",
                "content": TRANSCRIPT_PREFIX + transcript,
            }
        ],
    )
    return _message_text(response)
