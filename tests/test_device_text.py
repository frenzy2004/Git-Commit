from __future__ import annotations

from backend.config import get_settings
from backend.serial_out import _normalize_text4, _text_to_chunks, prepare_device_payload
from backend.text_utils import essentialize_text, split_simple_sentences


def test_text4_normalization_and_chunking():
    assert _normalize_text4("A red apple.\n") == "a red apple."
    assert _text_to_chunks("A red apple.") == ["a re", "d ap", "ple."]


def test_device_payload_defaults_to_text4(monkeypatch):
    monkeypatch.delenv("DEVICE_FORMAT", raising=False)
    monkeypatch.setenv("ENABLE_DEVICE_IO", "false")

    payload = prepare_device_payload("A red apple.", b"\x01\x02", settings=get_settings())

    assert payload["format"] == "text4"
    assert payload["enabled"] is False
    assert payload["sent"] is False
    assert payload["chunks"] == ["a re", "d ap", "ple."]


def test_essential_summary_limits_words():
    source = "One two three four five six seven eight nine ten eleven twelve thirteen fourteen fifteen sixteen seventeen eighteen nineteen."

    result = essentialize_text("image", source)

    assert len(result.split()) == 18
    assert result.endswith(".")


def test_sentence_splitter_returns_clean_parts():
    assert split_simple_sentences("First idea. Second idea?") == ["First idea.", "Second idea?"]
