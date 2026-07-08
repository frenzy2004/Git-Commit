from __future__ import annotations

from contextlib import asynccontextmanager

from fastapi import FastAPI, File, HTTPException, UploadFile
from fastapi.concurrency import run_in_threadpool
from fastapi.middleware.cors import CORSMiddleware

from .audio import warm_audio_model
from .braille import louis
from .config import Settings, get_settings
from .pipeline import process_audio_bytes, process_image_bytes
from .serial_out import serial_available
from .summarizer import summary_available
from .uploads import read_upload_limited, validate_audio_upload, validate_image_upload
from .vision import vision_available


def health_payload(settings: Settings, audio_ready: bool) -> dict:
    return {
        "status": "ok",
        "demo_mode": settings.mock_claude or settings.mock_transcribe,
        "device_transport": {
            "enabled": settings.enable_device_io,
            "mode": "serial" if settings.enable_device_io else "preview-only",
            "format": settings.device_format,
        },
        "services": {
            "vision": vision_available(settings),
            "lecture_summary": summary_available(settings),
            "transcription": settings.mock_transcribe or audio_ready,
            "braille": louis is not None,
            "serial": serial_available(),
            "device_preview": True,
        },
    }


def create_app(settings: Settings | None = None) -> FastAPI:
    settings = settings or get_settings()

    @asynccontextmanager
    async def lifespan(app: FastAPI):
        app.state.settings = settings
        app.state.audio_ready = settings.mock_transcribe
        if not settings.mock_transcribe:
            try:
                await run_in_threadpool(warm_audio_model, settings)
                app.state.audio_ready = True
            except Exception:
                app.state.audio_ready = False
        yield

    app = FastAPI(title="Fingertips Accessibility API", lifespan=lifespan)
    app.add_middleware(
        CORSMiddleware,
        allow_origins=list(settings.cors_origins),
        allow_methods=["GET", "POST", "OPTIONS"],
        allow_headers=["*"],
    )

    @app.get("/health")
    def health():
        return health_payload(settings, getattr(app.state, "audio_ready", settings.mock_transcribe))

    @app.post("/image")
    async def process_image(file: UploadFile = File(...)):
        media_type = validate_image_upload(file)
        image_bytes = await read_upload_limited(file, settings.max_image_bytes, "Image")

        try:
            return await run_in_threadpool(process_image_bytes, image_bytes, media_type, settings)
        except Exception as exc:
            raise HTTPException(503, f"Image pipeline unavailable: {exc}") from exc

    @app.post("/lecture")
    async def process_lecture(file: UploadFile = File(...)):
        suffix = validate_audio_upload(file)
        audio_bytes = await read_upload_limited(file, settings.max_audio_bytes, "Audio")

        try:
            return await run_in_threadpool(process_audio_bytes, audio_bytes, suffix, settings)
        except ValueError as exc:
            raise HTTPException(422, str(exc)) from exc
        except Exception as exc:
            raise HTTPException(503, f"Lecture pipeline unavailable: {exc}") from exc

    return app


app = create_app()
