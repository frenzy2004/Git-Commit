from __future__ import annotations

import base64

from .config import Settings, get_settings

try:
    from openai import OpenAI as OpenAIClient
except ImportError:  # pragma: no cover - live model calls are optional
    OpenAIClient = None


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

DEMO_IMAGE_NOTE = "The demo image shows a fig cut open so the seeds inside are visible."


def vision_available(settings: Settings | None = None) -> bool:
    active = settings or get_settings()
    return active.mock_openai or OpenAIClient is not None


def _client(settings: Settings):
    if OpenAIClient is None:
        raise RuntimeError("openai is not installed")
    if settings.openai_api_key:
        return OpenAIClient(api_key=settings.openai_api_key)
    return OpenAIClient()


def _image_data_url(image_bytes: bytes, media_type: str) -> str:
    encoded = base64.b64encode(image_bytes).decode("ascii")
    return f"data:{media_type};base64,{encoded}"


def summarize_image(
    image_bytes: bytes,
    media_type: str = "image/jpeg",
    settings: Settings | None = None,
) -> str:
    active = settings or get_settings()
    if active.mock_openai:
        return DEMO_IMAGE_NOTE

    response = _client(active).responses.create(
        model=active.vision_model,
        instructions=VISION_GUIDE,
        input=[
            {
                "role": "user",
                "content": [
                    {"type": "input_text", "text": VISION_ASK},
                    {"type": "input_image", "image_url": _image_data_url(image_bytes, media_type)},
                ],
            }
        ],
        max_output_tokens=512,
        text={"verbosity": "low"},
    )
    return response.output_text.strip()
