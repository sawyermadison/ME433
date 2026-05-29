import serial
import numpy as np
import matplotlib.pyplot as plt

# ============================================================
# USER SETTINGS
# ============================================================

PORT = "COM3"       # Change to your Pico port
BAUD = 115200
NUM_SAMPLES = 1000

# ============================================================
# CONNECT
# ============================================================

ser = serial.Serial(PORT, BAUD, timeout=5)

print("Waiting for Pico...")

# ============================================================
# WAIT FOR PROMPT
# ============================================================

while True:
    line = ser.readline().decode(errors='ignore').strip()

    if line:
        print(line)

    if "Enter the number of samples to read" in line:
        break

# ============================================================
# SEND SAMPLE COUNT
# ============================================================

ser.write(f"{NUM_SAMPLES}\n".encode())

print(f"Requested {NUM_SAMPLES} samples")

# ============================================================
# READ DATA
# ============================================================

time_data = []
raw_data = []
filtered_data = []

while len(time_data) < NUM_SAMPLES:

    line = ser.readline().decode(errors='ignore').strip()

    if not line:
        continue

    try:
        t, raw, filt = map(int, line.split('\t'))

        time_data.append(t/1000000)
        raw_data.append(raw)
        filtered_data.append(filt)

    except ValueError:
        # Ignore any startup messages
        print(line)

# ============================================================
# CONVERT TO NUMPY
# ============================================================

time_data = np.array(time_data)
raw_data = np.array(raw_data)
filtered_data = np.array(filtered_data)

# ============================================================
# SAMPLE RATE
# ============================================================

total_time = (time_data[-1] - time_data[0])

sample_rate = len(time_data) / total_time

print(f"Estimated sample rate: {sample_rate:.2f} Hz")

# ============================================================
# FFT RAW
# ============================================================

n = len(raw_data)

Y_raw = np.fft.fft(raw_data) / n
Y_raw = Y_raw[:n // 2]

frq = np.fft.fftfreq(n, d=1/sample_rate)
frq = frq[:n // 2]

# ============================================================
# FFT FILTERED
# ============================================================

Y_filtered = np.fft.fft(filtered_data) / n
Y_filtered = Y_filtered[:n // 2]

# ============================================================
# PLOT TIME DOMAIN
# ============================================================

fig1, (ax1, ax2) = plt.subplots(2, 1)

fig1.suptitle("HX711 Data")

ax1.plot(time_data, raw_data, label="Raw")
ax1.plot(time_data, filtered_data, label="Filtered")

ax1.set_xlabel("Time (s)")
ax1.set_ylabel("Counts")
ax1.legend()

# ============================================================
# PLOT FFT
# ============================================================

ax2.loglog(frq, np.abs(Y_raw), label="Raw FFT")
ax2.loglog(frq, np.abs(Y_filtered), label="Filtered FFT")

ax2.set_xlabel("Frequency (Hz)")
ax2.set_ylabel("|Y(f)|")
ax2.legend()

plt.tight_layout()
plt.show()

ser.close()