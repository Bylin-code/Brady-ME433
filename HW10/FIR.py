import numpy as np
import matplotlib.pyplot as plt
import csv
from scipy import signal

def load_signal(filename):
    """Load time and signal data from CSV file."""
    time_data = []
    signal_data = []
    with open(filename) as f:
        reader = csv.reader(f)
        for row in reader:
            time_data.append(float(row[0]))
            signal_data.append(float(row[1]))
    return np.array(time_data), np.array(signal_data)

def design_fir_lowpass(cutoff, numtaps, fs):
    """
    Design an FIR low-pass filter.
    
    Args:
        cutoff: Cutoff frequency in Hz
        numtaps: Number of filter coefficients (taps)
        fs: Sampling frequency in Hz
    
    Returns:
        Filter coefficients
    """
    nyq = 0.5 * fs
    normal_cutoff = cutoff / nyq
    return signal.firwin(numtaps, normal_cutoff)

def apply_fir_filter(signal_data, coefficients):
    """Apply FIR filter to the signal."""
    return signal.lfilter(coefficients, 1.0, signal_data)

def plot_signals(time_data, original_signal, filtered_signal, coefficients, cutoff, filename):
    """Plot original and filtered signals in separate subplots."""
    # Create figure with two subplots
    fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(10, 8))
    
    # Set the main title with filter parameters
    fig.suptitle(f'FIR Filter - {filename}\n' 
                f'Cutoff: {cutoff}Hz, Taps: {len(coefficients)}', 
                fontsize=12)
    
    # Plot original signal
    ax1.plot(time_data, original_signal, 'black')
    ax1.set_title('Original Signal')
    ax1.set_xlabel('Time (s)')
    ax1.set_ylabel('Amplitude')
    ax1.grid(True)
    
    # Plot filtered signal
    ax2.plot(time_data, filtered_signal, 'red')
    ax2.set_title('Filtered Signal')
    ax2.set_xlabel('Time (s)')
    ax2.set_ylabel('Amplitude')
    ax2.grid(True)
    
    plt.tight_layout()
    plt.show()

def main():
    # Parameters
    filename = 'sigD.csv'  # Change this to your signal file
    fs = 10000  # Sampling frequency in Hz
    
    # Tune these parameters:
    cutoff = 10  # Cutoff frequency in Hz - adjust this to filter different frequencies
    numtaps = 101  # Number of filter coefficients (more taps = sharper cutoff)
    
    # Load signal
    time_data, original_signal = load_signal(filename)
    
    # Design FIR filter
    coefficients = design_fir_lowpass(cutoff, numtaps, fs)
    
    # Apply filter
    filtered_signal = apply_fir_filter(original_signal, coefficients)
    
    # Plot results
    plot_signals(time_data, original_signal, filtered_signal, 
                coefficients, cutoff, filename)

if __name__ == "__main__":
    main()