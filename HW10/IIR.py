import matplotlib.pyplot as plt
import numpy as np
import csv

# Initialize lists to store time and data
time_data = []
signal_data = []
fil_signal_data = []

# Constants
point_num = 50
weightA = 0.1
weightB = 1 - weightA

# Read data from a single CSV file
filename = 'sigD.csv'  # Change this to the file you want to plot
with open(filename) as f:
    reader = csv.reader(f)
    for row in reader:
        time_data.append(float(row[0]))    # First column is time
        signal_data.append(float(row[1]))  # Second column is signal data

fil_signal_data = np.zeros_like(signal_data)
fil_signal_data[0] = signal_data[0]

for i in range(1, len(signal_data)):
    fil_signal_data[i] = weightA * signal_data[i] + weightB * fil_signal_data[i-1]


# Sampling rate and interval
Fs = 10000  # 10kHz sample rate
Ts = 1.0/Fs  # sampling interval

# Create figure with two subplots
fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(10, 8))
fig.suptitle(f'Signal Analysis: {filename} , A: {weightA}, B: {weightB}', fontsize=14)

# Time domain plot
ax1.plot(time_data, signal_data, 'black')
ax1.set_title('Original Signal')
ax1.set_xlabel('Time (s)')
ax1.set_ylabel('Amplitude')
ax1.grid(True)

ax2.plot(time_data, fil_signal_data, 'red')
ax2.set_title('Filtered Signal')
ax2.set_xlabel('Time (s)')
ax2.set_ylabel('Amplitude')
ax2.grid(True)

plt.tight_layout()
plt.show()