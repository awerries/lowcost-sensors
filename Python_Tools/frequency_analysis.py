#!/usr/bin/env python
"""This script processes any column of measurement and performs the Fast Fourier Transform (a faster DTFT). 
This is meant to provide insight for estimating the power spectral density of sensor measurements.

Adam Werries, 2016"""
import os
import sys
import numpy
import argparse

from matplotlib import pyplot as pplot
from time import mktime
from dateutil.parser import parse as parsetime 

def parse_datafile(filename, time_column, data_column):
    """Gets specified time_column and data_column numbers from a comma-separated file. Start counting at zero."""
    f = open(filename,'r')
    time = list()
    data = list()
    for line in f:
        line = line.split(',')
        if len(line) > 1:
            # Convert iso8601 time to unix time
            time.append(float(line[time_column]))
            # Convert string value to int
            data.append(float(line[data_column]))
    # Convert lists to numpy-arrays for convenience, 
    time = numpy.array(time)
    time = time - time.min()
    data = numpy.array(data)
    return (time, data)

def calculate_timestep(time):
    """Determine average spacing of time data (list). Data is assumed to be taken at a regular interval."""
    steps = numpy.diff(time)
    timestep = numpy.mean(steps[2:])
    stddev = numpy.std(steps[2:])
    timemax = numpy.max(steps[2:])
    timemin = numpy.min(steps[2:])
    print('Timesteps info [mean: {0}, std: {1}, min: {2}, max: {3}'.format(timestep, stddev, timemin, timemax))
    return timestep

def calculate_fft(timestep, data):
    """Performs FFT on time-signal, returning magnitudes at each frequency."""
    fourier = numpy.fft.rfft(data)
    fft_magnitude = numpy.absolute(fourier)
    frequency = numpy.fft.fftfreq(fft_magnitude.size, d = timestep)
    # Get rid of negative frequencies, they're not useful for visualization!
    # freq = list()
    # mag = list()
    # for i,f in enumerate(frequency):
        # if f >= 0:
            # freq.append(f)
            # mag.append(fft_magnitude[i])
    # return (numpy.array(freq), numpy.array(mag))
    return frequency, fft_magnitude

def plot_time(filename, time, data, title):
    """Plots time-based data."""
    pplot.figure()
    pplot.plot(time, data, 'bo')
    pplot.title(title)
    pplot.xlabel('Time (s)')
    pplot.ylabel('Sensor Reading')
    pplot.show()
    #pplot.savefig('{0}_time.png'.format(filename),dpi=300)

def plot_fft(filename, freq, mag, title):
    """Plots FFT data alongside 1/f (pink) and 1/f^2 (brown) noise curves"""
    pplot.figure()
    pplot.plot(freq,mag, 'bo', label='Magnitudes')
    print(max(mag))
    pplot.title(title)
    pplot.xlabel('Frequency (Hz)')
    pplot.xscale('log')
    pplot.xlim([1, 400])
    pplot.ylabel('Magnitude')
    pplot.yscale('log')
    pplot.ylim([.1, 1000])
    pplot.grid(True,which='both',ls='-',alpha=0.2)
    pplot.show()
    #pplot.savefig('{0}_fft.png'.format(filename),dpi=300)
    
def main():
    parser = argparse.ArgumentParser(description='Display FFT plot of a chosen time-column and data-column.')
    parser.add_argument('-t', '--time_column', type=int, help='Chosen column in file to use for time. Default is 0.', required=True)
    parser.add_argument('-d', '--data_column', type=int, help='Chosen column of data to compute FFT with. Default is 1.', required=True)
    parser.add_argument('filename', help='Sensor log filename.')
    args = parser.parse_args()
    time, data = parse_datafile(args.filename, args.time_column, args.data_column)
    filename, file_extension = os.path.splitext(args.filename)
    plot_time(filename, time, data, 'Time-domain for {0}, column {1}'.format(args.filename, args.data_column))
    timestep = calculate_timestep(time)
    frequency, fft_magnitude = calculate_fft(timestep, data) 
    plot_fft(filename, frequency, fft_magnitude, 'Frequency-domain of {0}, column {1}'.format(args.filename, args.data_column))

if __name__ == '__main__':
    main()

