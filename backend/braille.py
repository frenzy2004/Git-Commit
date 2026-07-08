from __future__ import annotations

import os

try:
    import louis
except ImportError:  # pragma: no cover - optional platform dependency
    louis = None


TABLES_BY_GRADE = {
    "1": ("en-us-g1.ctb",),
    "2": ("en-us-g2.ctb",),
}

ASCII_CELL_PAIRS = [
    (" ", 0x00),
    ("a", 0x01), ("b", 0x03), ("c", 0x09), ("d", 0x19), ("e", 0x11),
    ("f", 0x0B), ("g", 0x1B), ("h", 0x13), ("i", 0x0A), ("j", 0x1A),
    ("k", 0x05), ("l", 0x07), ("m", 0x0D), ("n", 0x1D), ("o", 0x15),
    ("p", 0x0F), ("q", 0x1F), ("r", 0x17), ("s", 0x0E), ("t", 0x1E),
    ("u", 0x25), ("v", 0x27), ("w", 0x3A), ("x", 0x2D), ("y", 0x3D),
    ("z", 0x35),
    (",", 0x02), (";", 0x06), (":", 0x12), (".", 0x32), ("!", 0x16),
    ("?", 0x26), ("-", 0x24), ("'", 0x04), ('"', 0x14), ("/", 0x0C),
]

FALLBACK_CELLS = dict(ASCII_CELL_PAIRS)
DIGIT_TO_LETTER = {"1": "a", "2": "b", "3": "c", "4": "d", "5": "e", "6": "f", "7": "g", "8": "h", "9": "i", "0": "j"}
BRAILLE_START = 0x2800
BRAILLE_END = 0x28FF
SIX_DOT_MASK = 0x3F


def active_tables() -> list[str]:
    grade = os.getenv("BRAILLE_GRADE", "2").strip()
    return list(TABLES_BY_GRADE.get(grade, TABLES_BY_GRADE["2"]))


def text_to_unicode_braille(text: str) -> str:
    if louis is None:
        raise RuntimeError("liblouis is not installed")
    return louis.translateString(active_tables(), text)


def _fallback_cell(character: str) -> int:
    normalized = character.lower()
    if normalized in DIGIT_TO_LETTER:
        normalized = DIGIT_TO_LETTER[normalized]
    return FALLBACK_CELLS.get(normalized, 0x00)


def _fallback_cells(text: str) -> bytes:
    return bytes(_fallback_cell(character) for character in text)


def _cell_from_braille_glyph(glyph: str) -> int | None:
    codepoint = ord(glyph)
    if BRAILLE_START <= codepoint <= BRAILLE_END:
        return codepoint & SIX_DOT_MASK
    if glyph in {" ", "\n"}:
        return 0x00
    return None


def text_to_cells(text: str) -> bytes:
    if louis is None:
        return _fallback_cells(text)

    cells = bytearray()
    for glyph in text_to_unicode_braille(text):
        cell = _cell_from_braille_glyph(glyph)
        if cell is not None:
            cells.append(cell)
    return bytes(cells)
