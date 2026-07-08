# Device Protocol

The default device protocol is `text4`.

## Frame Shape

Each frame is a four-character ASCII chunk followed by a newline:

```text
abcd\n
```

If the final chunk is shorter than four characters, the backend pads it with spaces.

## Supported Characters

The firmware maps lowercase letters, digits, spaces, and a small punctuation set to calibrated motor angles.

Recommended backend normalization:

```text
a-z 0-9 space ! ' , . - ? # ^
```

## Timing

The backend default is:

```env
SERIAL_CHUNK_DELAY_MS=0
```

This lets the ESP32 control the display hold time. The firmware currently waits after each chunk, releases the motors, then accepts the next serial line.

## Serial Defaults

```env
DEVICE_FORMAT=text4
SERIAL_BAUD=115200
```
