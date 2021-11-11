import os
import time
import serial
import argparse

parser = argparse.ArgumentParser()

parser.add_argument("input", help="the input file")
parser.add_argument("tty", help="the target tty")
parser.add_argument("-b",
                    "--baudrate",
                    help="serial baudrate, the default value is 115200",
                    default=115200)

args = parser.parse_args()

filename = args.input
port = args.tty
baudrate = args.baudrate


def bytes_from_file(filename, chunksize=8192):
    with open(filename, "rb") as f:
        chunk = f.read(chunksize)
        return chunk


with serial.Serial(port, baudrate) as ser:
    # ser.write("\n\nreboot\n".encode())
    time.sleep(2)
    ser.write("\n\nload\n".encode())

    size = os.path.getsize(filename)
    print(f'Image size is {size}')
    time.sleep(0.1)

    res = f"{size}\n"
    ser.write(res.encode())
    img = bytes_from_file(filename)
    time.sleep(0.1)
    ser.write(img)
    print("done")
