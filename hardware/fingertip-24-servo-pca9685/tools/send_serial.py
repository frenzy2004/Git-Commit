"""Send simple Fingertip serial commands to an Arduino.

Examples:
  python fingertip/tools/send_serial.py COM3 R
  python fingertip/tools/send_serial.py COM3 L:A
  python fingertip/tools/send_serial.py COM3 W:CUP
  python fingertip/tools/send_serial.py COM3 --spell CUP
"""

import argparse
import time

import serial


def send_line(ser: serial.Serial, line: str) -> None:
    print(f"> {line}")
    ser.write((line.strip() + "\n").encode("ascii"))
    ser.flush()
    time.sleep(0.15)
    while ser.in_waiting:
        print(ser.readline().decode("ascii", errors="replace").strip())


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("port", help="Arduino port, for example COM3")
    parser.add_argument("command", nargs="?", help="Command such as R, L:A, W:CUP, T:0")
    parser.add_argument("--spell", help="Spell a word one letter at a time on cell 1")
    parser.add_argument("--baud", type=int, default=9600)
    args = parser.parse_args()

    with serial.Serial(args.port, args.baud, timeout=1) as ser:
        time.sleep(2)

        if args.spell:
            for letter in args.spell.upper():
                if not letter.isalpha():
                    continue
                send_line(ser, "R")
                time.sleep(0.7)
                send_line(ser, f"L:{letter}")
                time.sleep(1.5)
            send_line(ser, "R")
            return

        if not args.command:
            raise SystemExit("Give a command like R, L:A, W:CUP, or use --spell WORD")

        send_line(ser, args.command.upper())


if __name__ == "__main__":
    main()
