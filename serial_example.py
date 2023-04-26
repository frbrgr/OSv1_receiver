#!/usr/bin/env python3

import serial

# This script shows how to receive temperature data via serial.
# First it checks for the welcome message since the Arduino always
# resets when a serial connection is established. Then we read lines
# until a valid temperature is received, skipping error messages.

with serial.Serial('/dev/ttyUSB0', 19200, timeout=60) as port:
    if port.readline() == b'OSv1_receiver\r\n':
        done = False
        while not done:
            line = port.readline()
            if line.startswith(b'E'):  # error message
                continue
            else:
                # not an error message, do something with the temperature
                print(line.decode("utf-8").strip())
                done = True
    else:
        print("something went wrong")
