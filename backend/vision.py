from __future__ import annotations

import base64
import importlib
import importlib.util

from .config import Settings, get_settings


VISION_GUIDE = (
    "Act as a classroom access adapter for a deaf-blind learner. "
    "Your input is one classroom image: a page, slide, board, diagram, object, or scene. "
    "Return plain ASCII text only. Do not use markdown, list formatting, emoji, or decorative wording. "
    "Keep the answer short enough for a tactile display. "
    "For pages and slides, explain the meaning instead of copying every word. "
    "For diagrams, name the important parts and the connection between them. "
    "For ordinary scenes or objects, give one direct sentence about what is visible. "
    "Prefer the central learning point over labels, styling, background detail, and uncertain guesses."
)

VISION_ASK = (
    "Convert this image into the shortest useful learning note for tactile reading. "
    "Use one sentence when possible and never more than two short sentences."
)

DEMO_IMAGE_NOTE = "This picture has one important idea. Turn off MOCK_CLAUDE to analyze real images."


def _anthropic_installed() -> bool:
    return importlib.util.find_spec("anthropic") is not None


def vision_available(settings: Settings | None = None) -> bool:
    active = settings or get_settings()
    return active.mock_claude or _anthropic_installed()


def _client(settings: Settings):
    if not _anthropic_installed():
        raise RuntimeError("anthropic is not installed")
    client_type = importlib.import_module("anthropic").Anthropic
    if settings.anthropic_api_key:
        return client_type(api_key=settings.anthropic_api_key)
    return client_type()


def _encoded_source(image_bytes: bytes, media_type: str) -> dict:
    return {
        "type": "image",
        "source": {
            "type": "base64",
            "media_type": media_type,
            "data": base64.b64encode(image_bytes).decode("ascii"),
        },
    }


def _plain_text(message) -> str:
    output = []
    for block in getattr(message, "content", []):
        if getattr(block, "type", None) == "text":
            output.append(block.text)
    return "".join(output).strip()


def summarize_image(
    image_bytes: bytes,
    media_type: str = "image/jpeg",
    settings: Settings | None = None,
) -> str:
    active = settings or get_settings()
    if active.mock_claude:
        return DEMO_IMAGE_NOTE

    result = _client(active).messages.create(
        model=active.vision_model,
        max_tokens=512,
        system=VISION_GUIDE,
        messages=[
            {
                "role": "user",
                "content": [
                    _encoded_source(image_bytes, media_type),
                    {"type": "text", "text": VISION_ASK},
                ],
            }
        ],
    )
    return _plain_text(result)
