#!/usr/bin/env python

"""Write output.rom to EEPROM via Serial."""
import itertools
import sys
import time

import serial


USB_TTY = "/dev/ttyUSB0"
BAUD_RATE = 57600
NUM_PAGES = 10
BYTES_PER_PAGE = 64


# https://docs.python.org/3/library/itertools.html#itertools-recipes
def grouper(iterable, n, fillvalue=None):
    args = [iter(iterable)] * n
    return itertools.zip_longest(*args, fillvalue=fillvalue)


def main():

    if len(sys.argv) != 2:
        print(f"Usage: {sys.argv[0]} <data.rom>")
        sys.exit(1)

    ser = serial.Serial(USB_TTY, BAUD_RATE, timeout=None, exclusive=False)
    time.sleep(2)  # delay needed to successfully connect

    with open('output.rom', 'rb') as f:
        out = f.read()

    assert len(out) == BYTES_PER_PAGE * NUM_PAGES

    start = time.time()
    for page in grouper(out, BYTES_PER_PAGE):
        ser.write(page)
    print(f'Elapsed time: {time.time() - start}')


if __name__ == "__main__":
    main()
