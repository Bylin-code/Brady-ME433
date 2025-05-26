import matplotlib.pyplot as plt
import numpy as np
import csv

# Initialize lists to store time and data
time_data = []
signal_data = []
fil_signal_data = []

# Constants
point_num = 50

# Read data from a single CSV file
filename = 'sigD.csv'  # Change this to the file you want to plot
with open(filename) as f:
    reader = csv.reader(f)
    for row in reader:
        time_data.append(float(row[0]))    # First column is time
        signal_data.append(float(row[1]))  # Second column is signal data

for i in range(len(signal_data)):
    if i < point_num:
        del signal_data[0]
        del time_data[0]
        continue
    else:
        fil_signal_data.append(np.mean(signal_data[i-point_num:i]))



# Sampling rate and interval
Fs = 10000  # 10kHz sample rate
Ts = 1.0/Fs  # sampling interval

# Create figure with two subplots
fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(10, 8))
fig.suptitle(f'Signal Analysis: {filename} , number of pts: {point_num}', fontsize=14)

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