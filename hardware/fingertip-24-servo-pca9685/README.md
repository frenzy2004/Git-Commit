# Fingertip 24-Servo PCA9685 Hardware Package

This folder contains the Arduino sketches and bench notes for the 24-servo tactile fingertip prototype.

The current working hardware is:

- Arduino Nano over USB serial.
- Two PCA9685 16-channel PWM servo boards.
- PCA9685 board 1 at I2C address `0x40`.
- PCA9685 board 2 at I2C address `0x41` with `A0` bridged.
- 24 SG90/TowerPro-style micro servos.
- External servo power rail. Do not power the full servo array from Nano USB.

No infinite demo runner is included. The firmware has the required Arduino `loop()` function, but demos only run after explicit serial commands.

## Folder Contents

```text
hardware/fingertip-24-servo-pca9685/
  README.md
  TROUBLESHOOTING.md
  arduino/
    phase1_single_servo/
    phase2_one_cell_pca9685/
    fingertip_two_board_calibrate/
    fingertip_two_board_complete/
    fingertip_24_servo/
    continuous_24_servo_test/
    smooth_board1_6_servo_test/
    gentle_channel0_pulse_test/
    hold_channel0_board1/
    phase2_channel0_power_debug/
    i2c_scanner/
    website_two_board_sweep/
  tools/
    send_serial.py
```

## Sketch Inventory

| Sketch | Purpose |
| --- | --- |
| `phase1_single_servo` | Direct Nano D9 single-servo test. Used before PCA9685. |
| `phase2_one_cell_pca9685` | One PCA9685 board, one 6-dot cell on channels 0-5. |
| `fingertip_two_board_calibrate` | Current calibration and demo firmware. Command-driven, no auto movement on boot. |
| `fingertip_two_board_complete` | Two-board command firmware with direct `T`, `L`, and reset commands. |
| `fingertip_24_servo` | Early 24-servo firmware. Kept for historical reference. |
| `continuous_24_servo_test` | One-at-a-time full channel sweep. |
| `smooth_board1_6_servo_test` | Smooth loop over board 1 channels 0-5. |
| `gentle_channel0_pulse_test` | Gentle board 1 channel 0 pulse test. |
| `hold_channel0_board1` | Move/hold channel 0 for power and horn inspection. |
| `phase2_channel0_power_debug` | Board 1 channel 0 power debug. |
| `i2c_scanner` | Finds PCA9685 boards on the I2C bus. |
| `website_two_board_sweep` | External reference-style two-board sweep sketch. |

## Recommended Current Firmware

Use:

```text
arduino/fingertip_two_board_calibrate/fingertip_two_board_calibrate.ino
```

Why:

- It does not move servos on startup.
- It supports direct single-channel pulse commands.
- It contains the calibrated safe rhythm command `RHY`.
- It keeps demos finite and serial-triggered.

## Wiring

### Nano to PCA9685 Logic

```text
Nano A4  -> PCA9685 SDA
Nano A5  -> PCA9685 SCL
Nano 5V  -> PCA9685 VCC
Nano GND -> PCA9685 GND
```

For two PCA9685 boards, chain logic pin-to-pin:

```text
SDA -> SDA
SCL -> SCL
VCC -> VCC
GND -> GND
```

Board 2 must have `A0` bridged so it appears at `0x41`.

### Servo Power

The green screw terminal is servo power only:

```text
Power supply + -> PCA9685 V+
Power supply - -> PCA9685 GND
```

Both boards need access to the same servo power rail. The Nano ground, PCA grounds, and power-supply negative must share common ground.

Do not connect PCA9685 green-terminal `V+` to Nano `5V`. `VCC` is logic power; `V+` is servo power.

### Servo Plug Orientation

On the PCA9685 servo headers:

```text
Orange / yellow / white = signal
Red                     = V+
Brown / black           = GND
```

If a servo buzzes but does not move, check plug orientation and the exact channel column first.

## I2C Scan Expectations

The scanner should find:

```text
0x40  board 1
0x41  board 2
```

Some PCA9685 boards also expose all-call address `0x70`. That can appear or disappear depending on board/library behavior and is not the primary identity for either board.

## Global Channel Map

The firmware addresses servos as global channels `T0` to `T23`.

| Global | Board | PCA channel | Cell | Dot |
| --- | --- | --- | --- | --- |
| `T0` | `0x40` | 0 | 1 | 1 |
| `T1` | `0x40` | 1 | 1 | 2 |
| `T2` | `0x40` | 2 | 1 | 3 |
| `T3` | `0x40` | 3 | 1 | 4 |
| `T4` | `0x40` | 4 | 1 | 5 |
| `T5` | `0x40` | 5 | 1 | 6 |
| `T6` | `0x40` | 6 | 2 | 1 |
| `T7` | `0x40` | 7 | 2 | 2 |
| `T8` | `0x40` | 8 | 2 | 3 |
| `T9` | `0x40` | 9 | 2 | 4 |
| `T10` | `0x40` | 10 | 2 | 5 |
| `T11` | `0x40` | 11 | 2 | 6 |
| `T12` | `0x41` | 0 | 3 | 1 |
| `T13` | `0x41` | 1 | 3 | 2 |
| `T14` | `0x41` | 2 | 3 | 3 |
| `T15` | `0x41` | 3 | 3 | 4 |
| `T16` | `0x41` | 4 | 3 | 5 |
| `T17` | `0x41` | 5 | 3 | 6 |
| `T18` | `0x41` | 6 | 4 | 1 |
| `T19` | `0x41` | 7 | 4 | 2 |
| `T20` | `0x41` | 8 | 4 | 3 |
| `T21` | `0x41` | 9 | 4 | 4 |
| `T22` | `0x41` | 10 | 4 | 5 |
| `T23` | `0x41` | 11 | 4 | 6 |

## Serial Commands In Current Calibration Firmware

Baud rate:

```text
9600
```

Commands:

```text
S             Print I2C status and selected channel.
C:9           Select global channel 9.
P:320         Set selected channel to pulse 320.
X:9:340       Set global channel 9 directly to pulse 340.
+             Nudge selected channel up by 5 pulse units.
-             Nudge selected channel down by 5 pulse units.
O1            Cell 1 opposite-direction demo.
RHY           Full T0-T23 safe rhythm demo.
?             Print selected channel and current selected pulse.
```

## Current Calibration Findings

These values came from live bench tests on the glued cardboard/foam prototype. They are not universal servo values.

Important concept:

```text
pulse number = target position
step size + delay = motion speed
```

A jump like `300 -> 500` is fast and risky. A stepped move like `300 -> 305 -> 310` is controlled.

### Current Safe Bases

| Channel | Base / normal | Tested movement |
| --- | ---: | --- |
| `T0` | 420 | Controlled sweep worked. |
| `T1` | 420 | Cell 1 mirrored direction group. |
| `T2` | 420 | Cell 1 mirrored direction group. |
| `T3` | 420 | Must be treated with mirrored direction, not like early T0 grouping. |
| `T4` | 420 | Cell 1 mirrored direction group. |
| `T5` | 420 | Cell 1 mirrored direction group. |
| `T6` | 300 | `300 -> 320 -> 300` tested. |
| `T7` | 300 | `300 -> 320 -> 300` tested. |
| `T8` | 300 | `300 -> 280 -> 300` tested. |
| `T9` | 320 | Normal is `320`; up is higher, e.g. `320 -> 340 -> 320`. Do not reset to `300` unless intentionally lowering. |
| `T10` | 300 | `300 -> 320 -> 300` tested. |
| `T11` | 300 | `300 -> 320 -> 300` tested. |
| `T12` | 300 | `300 -> 320 -> 300` tested. |
| `T13` | 300 | `300 -> 320 -> 300` tested. |
| `T14` | 300 | `300 -> 320 -> 300` tested. |
| `T15` | 300 | `300 -> 320 -> 300` tested. |
| `T16` | 300 | `300 -> 320 -> 300` tested. |
| `T17` | 300 | `300 -> 320 -> 300` tested. |
| `T18` | 300 | `300 -> 320 -> 300` tested. |
| `T19` | 300 | `300 -> 320 -> 300` tested. |
| `T20` | 300 | `300 -> 320 -> 300` tested. |
| `T21` | 300 | Included conservatively in full rhythm only. |
| `T22` | 300 | Included conservatively in full rhythm only. |
| `T23` | 300 | Included conservatively in full rhythm only. |

### Current Rhythm Table

`RHY` uses a 10-pulse amplitude and one channel at a time.

```text
T0  base 420, direction -10
T1  base 420, direction +10
T2  base 420, direction +10
T3  base 420, direction +10
T4  base 420, direction +10
T5  base 420, direction +10

T6  base 300, direction +10
T7  base 300, direction +10
T8  base 300, direction -10
T9  base 320, direction +10
T10 base 300, direction +10
T11 base 300, direction +10

T12 base 300, direction +10
T13 base 300, direction +10
T14 base 300, direction +10
T15 base 300, direction +10
T16 base 300, direction +10
T17 base 300, direction +10

T18 base 300, direction +10
T19 base 300, direction +10
T20 base 300, direction +10
T21 base 300, direction +10
T22 base 300, direction +10
T23 base 300, direction +10
```

`RHY` timing:

```text
3 seconds rhythm
3 seconds pause
3 seconds rhythm
1 second pause
reset touched servos to base
```

## Upload Commands

Using Arduino CLI from the original bench machine:

```powershell
.\.tools\arduino-cli\arduino-cli.exe compile --fqbn arduino:avr:nano:cpu=atmega328 hardware\fingertip-24-servo-pca9685\arduino\fingertip_two_board_calibrate
.\.tools\arduino-cli\arduino-cli.exe upload -p COM3 --fqbn arduino:avr:nano:cpu=atmega328 hardware\fingertip-24-servo-pca9685\arduino\fingertip_two_board_calibrate
```

If upload fails with `not in sync`, check cable/port first. Some Nano clones may require:

```powershell
arduino:avr:nano:cpu=atmega328old
```

The working bench mostly used the normal bootloader target:

```text
arduino:avr:nano:cpu=atmega328
```

## Safe Demo Policy

The hardware is mechanically sensitive because the rods are taped/glued to cardboard and foam. Avoid full-speed jumps.

Safe:

```text
small amplitude: 5-20 pulse units
slow steps: 5 pulse units per step
delay: 450-650 ms during calibration
one channel at a time for unknown channels
```

Risky:

```text
large jumps like 420 -> 120
moving all 24 channels at once
assuming every servo has the same direction
assuming every servo uses the same neutral
```

## Notes On Arduino `loop()`

Arduino sketches must include a `loop()` function. In the current calibration firmware, `loop()` only reads serial input. It does not start a rhythm by itself.

The rhythm is triggered only by sending:

```text
RHY
```

That command is finite. It is not an infinite loop.
