from __future__ import annotations

import os

from fastapi import HTTPException, UploadFile


ALLOWED_IMAGE_TYPES = {"image/jpeg", "image/png", "image/webp", "image/gif"}
ALLOWED_AUDIO_TYPES = {
    "audio/webm",
    "audio/wav",
    "audio/x-wav",
    "audio/mpeg",
    "audio/mp3",
    "audio/ogg",
    "audio/mp4",
    "audio/x-m4a",
    "audio/aac",
}
ALLOWED_AUDIO_EXTENSIONS = {".webm", ".wav", ".mp3", ".ogg", ".mp4", ".m4a", ".aac"}


def validate_image_upload(file: UploadFile) -> str:
    content_type = (file.content_type or "").lower()
    if content_type not in ALLOWED_IMAGE_TYPES:
        raise HTTPException(400, f"Unsupported image type: {file.content_type}")
    return content_type


def validate_audio_upload(file: UploadFile) -> str:
    filename = (file.filename or "").lower()
    extension = os.path.splitext(filename)[1]
    content_type = (file.content_type or "").lower()
    if content_type not in ALLOWED_AUDIO_TYPES and extension not in ALLOWED_AUDIO_EXTENSIONS:
        raise HTTPException(400, f"Unsupported audio type: {file.content_type}")
    return extension or ".webm"


async def read_upload_limited(file: UploadFile, max_bytes: int, label: str) -> bytes:
    data = bytearray()
    while True:
        chunk = await file.read(1024 * 1024)
        if not chunk:
            break
        data.extend(chunk)
        if len(data) > max_bytes:
            raise HTTPException(413, f"{label} too large (max {max_bytes // (1024 * 1024)} MB)")
    return bytes(data)
