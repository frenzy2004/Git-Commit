from __future__ import annotations

import re


def split_simple_sentences(text: str) -> list[str]:
    cleaned = text.strip()
    if not cleaned:
        return []
    parts = [part.strip() for part in re.split(r"(?<=[.!?])\s+", cleaned) if part.strip()]
    return parts or [cleaned]


def trim_words(text: str, max_words: int) -> str:
    words = text.split()
    if len(words) <= max_words:
        return text.strip()

    trimmed = " ".join(words[:max_words]).rstrip(",;:-")
    if trimmed and trimmed[-1] not in ".!?":
        trimmed += "."
    return trimmed


def essentialize_text(mode: str, text: str) -> str:
    cleaned = re.sub(r"\s+", " ", text).strip()
    if not cleaned:
        return cleaned

    sentences = split_simple_sentences(cleaned)
    if mode == "image":
        return trim_words(" ".join(sentences[:1]), 18)

    return trim_words(" ".join(sentences[:2]), 28)
