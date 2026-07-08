# Troubleshooting Guide

This guide documents the failures seen during the bench build and the fixes that worked.

## Golden Rule

Do not debug the whole 24-servo system at once.

Use this order:

```text
1. I2C scan
2. One known-good channel
3. One unknown channel
4. One cell
5. Multi-cell rhythm
6. Full rhythm
```

## Symptom: PCA9685 Board Not Detected

Expected I2C scan:

```text
0x40
0x41
```

If only `0x40` appears:

- Board 2 `A0` bridge may be missing or bad.
- Board 2 SDA/SCL may not be chained.
- Board 2 VCC/GND logic power may be missing.
- SDA/SCL may be swapped.

If neither board appears:

- Nano A4/A5 wiring is wrong.
- Nano GND is not common with PCA GND.
- PCA VCC is missing.
- The board is held in reset/disabled or wiring is shorted.

`0x70` can appear as PCA9685 all-call. It is not board 1 or board 2.

## Symptom: Servo Buzzes But Does Not Move

Most likely causes:

- Servo rail voltage is sagging.
- Servo plug is reversed.
- Signal wire is not on the signal row.
- Rod or pin is mechanically binding.
- Horn is glued or taped with preload.
- The pulse target is trying to push through the board or guide.

Debug action:

```text
1. Stop full demos.
2. Test the single channel with X:n:pulse.
3. Move in 5-pulse increments only.
4. Listen for quiet neutral.
5. Mark that neutral in the calibration table.
```

## Symptom: One Servo Moves Opposite Direction

This is expected when servos are mounted mirrored.

Do not fix this by rewiring the PCA. Fix it in calibration:

```text
normal channel:   base -> base + amount
mirrored channel: base -> base - amount
```

The `RHYTHM_DIRECTION[]` table in `fingertip_two_board_calibrate.ino` exists for this reason.

Known mirrored/special cases from the bench:

```text
T0 uses negative direction from base 420
T8 uses negative direction from base 300
T9 normal is 320 and up is positive
```

## Symptom: Movement Is Erratic

Common causes:

- Large pulse jumps.
- Too many servos moving simultaneously.
- Weak battery pack.
- Rods flexing sideways.
- Missing rigid guide plate.
- Servo deadband: tiny changes do nothing, then movement appears suddenly.

Fixes:

- Use stepped movement.
- Use one servo at a time.
- Use `RHY` instead of manual repeated serial commands for demos.
- Keep amplitude around 10 pulse units for full-system demos.
- Add a printed/rigid guide plate so rods move vertically.

## Symptom: Nothing Moves But Serial Looks Good

If serial says:

```text
I2C 0x40 board1: FOUND
I2C 0x41 board2: FOUND
SET global=...
```

then the Nano-to-PCA command path is working.

Focus on:

- Servo power.
- Common ground.
- Servo plug orientation.
- Channel column.
- Physical binding.

## Symptom: Upload Fails With `not in sync`

Observed error:

```text
programmer is not responding
not in sync: resp=0x00
```

Fix checklist:

- Close any serial monitor using the port.
- Unplug/replug Nano USB.
- Recheck COM port with `arduino-cli board list`.
- Try normal Nano bootloader first:

```text
arduino:avr:nano:cpu=atmega328
```

- If needed, try old bootloader:

```text
arduino:avr:nano:cpu=atmega328old
```

## Symptom: COM Port Disappears

Seen during bench testing:

```text
Could not find file 'COM3'
No boards found
```

Likely causes:

- USB cable momentarily disconnected.
- Nano reset/re-enumerated.
- Brownout/noisy power coupling through ground.

Fix:

```text
1. Stop sending commands.
2. Run arduino-cli board list.
3. Wait a few seconds and scan again.
4. Reopen serial only after the port returns.
```

## Power Notes

USB power is for the Nano logic only.

Servo power must come from an external 5V rail with enough current. A 4xAA pack can prove motion but may sag under many SG90s.

Recommended for full bench demos:

```text
5V regulated supply
5A or more
common ground with Nano and PCA boards
shorter/thicker servo power wires if possible
```

Do not use 9V on SG90 servos.

## Mechanical Notes

The prototype shown during calibration used:

- Cardboard base.
- Foam/cardboard risers.
- Taped/glued SG90 servos.
- Long vertical rods.
- No final rigid guide plate yet.

That means the rods can lean and bind. A final guide plate with smooth holes is not optional if the goal is reliable raised-dot motion.

Recommended mechanical improvement:

```text
1. Add a rigid top guide plate.
2. Keep rods vertical.
3. Reduce side load on horns.
4. Avoid preloading the servo horn at neutral.
5. Use rounded tactile pin tops.
```

## Safe Manual Test Patterns

Single channel with positive direction:

```text
X:12:305
X:12:310
X:12:315
X:12:320
X:12:315
X:12:310
X:12:305
X:12:300
```

Single channel with negative direction:

```text
X:8:295
X:8:290
X:8:285
X:8:280
X:8:285
X:8:290
X:8:295
X:8:300
```

T9 special case:

```text
X:9:325
X:9:330
X:9:335
X:9:340
X:9:335
X:9:330
X:9:325
X:9:320
```

## Before Any Public Demo

Run:

```text
S
```

Confirm:

```text
I2C 0x40 board1: FOUND
I2C 0x41 board2: FOUND
```

Then run one finite demo:

```text
RHY
```

Do not start an unattended infinite serial loop.
