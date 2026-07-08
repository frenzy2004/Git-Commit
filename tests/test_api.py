from __future__ import annotations

import pytest
from fastapi.testclient import TestClient

from backend.config import get_settings
from backend.main import create_app


@pytest.fixture()
def client(monkeypatch):
    monkeypatch.setenv("MOCK_OPENAI", "true")
    monkeypatch.setenv("MOCK_TRANSCRIBE", "true")
    monkeypatch.delenv("OPENAI_API_KEY", raising=False)
    monkeypatch.setenv("ENABLE_DEVICE_IO", "false")
    monkeypatch.setenv("DEVICE_FORMAT", "text4")
    monkeypatch.setenv("MAX_IMAGE_BYTES", str(10 * 1024 * 1024))
    monkeypatch.setenv("MAX_AUDIO_BYTES", str(50 * 1024 * 1024))

    with TestClient(create_app(get_settings())) as api:
        yield api


def test_health_reports_demo_preview_text4(client):
    response = client.get("/health")

    assert response.status_code == 200
    payload = response.json()
    assert payload["status"] == "ok"
    assert payload["demo_mode"] is True
    assert payload["device_transport"] == {
        "enabled": False,
        "mode": "preview-only",
        "format": "text4",
    }


def test_image_endpoint_keeps_frontend_contract(client):
    response = client.post(
        "/image",
        files={"file": ("page.png", b"\x89PNG\r\n\x1a\n", "image/png")},
    )

    assert response.status_code == 200
    payload = response.json()
    assert payload["mode"] == "image"
    assert payload["summary"] == payload["simple_text"]
    assert payload["simple_sentences"]
    assert payload["device"]["format"] == "text4"
    assert payload["serial"] == payload["device"]


def test_lecture_endpoint_keeps_frontend_contract(client):
    response = client.post(
        "/lecture",
        files={"file": ("lecture.webm", b"\x1aE\xdf\xa3", "audio/webm")},
    )

    assert response.status_code == 200
    payload = response.json()
    assert payload["mode"] == "lecture"
    assert payload["transcript"]
    assert len(payload["simple_sentences"]) == 2
    assert payload["device"]["chunks"]


def test_realtime_session_endpoint_returns_sdp(monkeypatch):
    monkeypatch.setenv("OPENAI_API_KEY", "test-key")
    monkeypatch.setenv("OPENAI_REALTIME_MODEL", "gpt-realtime-2")

    def fake_realtime_call(offer_sdp, settings):
        assert offer_sdp == "v=0"
        assert settings.realtime_model == "gpt-realtime-2"
        return "v=0\r\nanswer"

    monkeypatch.setattr("backend.main.create_realtime_call", fake_realtime_call)

    with TestClient(create_app(get_settings())) as api:
        response = api.post(
            "/realtime/session",
            content="v=0",
            headers={"content-type": "application/sdp"},
        )

    assert response.status_code == 200
    assert response.headers["content-type"].startswith("application/sdp")
    assert response.text == "v=0\r\nanswer"


def test_realtime_session_requires_sdp(client):
    response = client.post(
        "/realtime/session",
        content="",
        headers={"content-type": "application/sdp"},
    )

    assert response.status_code == 400
    assert response.json()["detail"] == "Missing SDP offer"


def test_rejects_oversized_upload_before_pipeline(monkeypatch):
    monkeypatch.setenv("MOCK_OPENAI", "true")
    monkeypatch.setenv("MOCK_TRANSCRIBE", "true")
    monkeypatch.setenv("MAX_IMAGE_BYTES", "4")

    with TestClient(create_app(get_settings())) as api:
        response = api.post(
            "/image",
            files={"file": ("too-big.png", b"12345", "image/png")},
        )

    assert response.status_code == 413
    assert "too large" in response.json()["detail"]
