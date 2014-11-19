import serial
import sys
import threading
import time

sp = serial.Serial('COM5', 115200, timeout=0.1, writeTimeout=None)

BUF_SIZE = 512

def read_ts():
    while True:
        s = sys.stdin.readline()
        if not s:
            break
        # time.sleep(0.1)
        sp.write(s)

threading.Thread(target=read_ts).start()

try:
    while True:
        dat = sp.read(BUF_SIZE)
        sys.stdout.write(dat)
        sys.stdout.flush()
except KeyboardInterrupt:
    pass