import serial
import matplotlib.pyplot as plt
import matplotlib.animation as animation
from collections import deque
import sys

# ---------------- CONFIG ----------------
PORT = "COM6"   # Your Bluetooth COM port
BAUD = 115200
WINDOW_SIZE = 1000

# ---------------- SERIAL CONNECTION ----------------
try:
    ser = serial.Serial(PORT, BAUD, timeout=1)
    print(f"Connected to {PORT}")
except Exception as e:
    print("Could not open Bluetooth COM port.")
    print(e)
    sys.exit()

# ---------------- DATA BUFFER ----------------
data = deque([0]*WINDOW_SIZE, maxlen=WINDOW_SIZE)

# ---------------- PLOT SETUP ----------------
fig, ax = plt.subplots()
line, = ax.plot(data)

ax.set_ylim(0, 4095)
ax.set_xlim(0, WINDOW_SIZE)
ax.set_title("Bluetooth Live ECG")
ax.set_xlabel("Samples")
ax.set_ylabel("ADC Value")
ax.grid(True)

def update(frame):
    while ser.in_waiting:
        try:
            value = int(ser.readline().decode().strip())
            data.append(value)
        except:
            pass

    line.set_ydata(data)
    return line,

ani = animation.FuncAnimation(fig, update, interval=20)

plt.show()

ser.close()
