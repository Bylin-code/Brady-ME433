import matplotlib.pyplot as plt
import numpy as np
import csv

dt = 1.0/10000.0 # 10kHz

tA = []
tB = []
tC = []
tD = []

dataA = []
dataB = []
dataC = []
dataD = []

sigs = ['sigA.csv', 'sigB.csv', 'sigC.csv', 'sigD.csv']

for sig in sigs:
    with open(sig) as f:
        # open the csv file
        reader = csv.reader(f)
        for row in reader:
            # read the rows 1 one by one
            if sig == 'sigA.csv':
                tA.append(float(row[0])) # leftmost column
                dataA.append(float(row[1])) # second column
            elif sig == 'sigB.csv':
                tB.append(float(row[0]))
                dataB.append(float(row[1]))
            elif sig == 'sigC.csv':
                tC.append(float(row[0]))
                dataC.append(float(row[1]))
            elif sig == 'sigD.csv':
                tD.append(float(row[0]))
                dataD.append(float(row[1]))

# FFT Constants
Fs = 10000  # sample rate
Ts = 1.0/Fs  # sampling interval

# Process and plot each signal in separate figures
signals = [
    # Make tuples of (signal_id, time, data, title)
    ('A', tA, dataA, 'Signal A'),
    ('B', tB, dataB, 'Signal B'),
    ('C', tC, dataC, 'Signal C'),
    ('D', tD, dataD, 'Signal D')
]

# For loops through each signal tuple and access each element of it and gives them a name
# ordered and immutable
# not indexed, i have to keep track of their order in this for loop and 
for sig_id, t, y, title in signals:
    # Create a new figure for each signal
    fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(10, 8))
    fig.suptitle(title, fontsize=14)
    
    # Time domain plot
    ax1.plot(t, y, 'red')
    ax1.set_title('Time Domain')
    ax1.set_xlabel('Time (s)')
    ax1.set_ylabel('Amplitude')
    ax1.grid(True)
    
    # Frequency domain plot
    n = len(y)
    k = np.arange(n)
    T = n/Fs
    frq = k/T
    frq = frq[range(int(n/2))]  # one side frequency range
    Y = np.fft.fft(y)/n  # FFT computing and normalization
    Y = Y[range(int(n/2))]
    
    ax2.loglog(frq, abs(Y), 'black')  # 'k' for black
    ax2.set_title('Frequency Domain')
    ax2.set_xlabel('Freq (Hz)')
    ax2.set_ylabel('|Y(freq)|')
    ax2.grid(True)
    
    plt.tight_layout()
    plt.show()