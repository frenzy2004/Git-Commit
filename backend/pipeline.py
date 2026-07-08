from __future__ import annotations

from .braille import text_to_cells
from .config import Settings
from .serial_out import prepare_device_payload
from .summarizer import summarize_transcript
from .text_utils import essentialize_text, split_simple_sentences
from .vision import summarize_image
from .audio import transcribe


def build_response(
    mode: str,
    simple_text: str,
    cells: bytes,
    device_payload: dict,
    transcript: str | None = None,
) -> dict:
    final_text = essentialize_text(mode, simple_text)
    response = {
        "mode": mode,
        "simple_text": final_text,
        "simple_sentences": split_simple_sentences(final_text),
        "summary": final_text,
        "cell_count": len(cells),
        "braille_preview": device_payload["preview"],
        "device": device_payload,
        "serial": device_payload,
    }
    if transcript is not None:
        response["transcript"] = transcript
    return response


def process_image_bytes(image_bytes: bytes, media_type: str, settings: Settings) -> dict:
    simple_text = summarize_image(image_bytes, media_type=media_type, settings=settings)
    cells = text_to_cells(simple_text)
    device_payload = prepare_device_payload(simple_text, cells, settings=settings)
    return build_response("image", simple_text, cells, device_payload)


def process_audio_bytes(audio_bytes: bytes, suffix: str, settings: Settings) -> dict:
    transcript = transcribe(audio_bytes, suffix=suffix, settings=settings)
    if not transcript:
        raise ValueError("Could not transcribe audio; check the recording")

    simple_text = summarize_transcript(transcript, settings=settings)
    cells = text_to_cells(simple_text)
    device_payload = prepare_device_payload(simple_text, cells, settings=settings)
    return build_response("lecture", simple_text, cells, device_payload, transcript=transcript)
