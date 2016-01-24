#!/usr/bin/env python
import statistics
import re
""" Code parses output from Sparkfun Razor IMU and writes to a matlab-friendly csv file.
Author: Adam Werries, awerries@cmu.edu

Pre-parsed format is user-specified, as the Razor is essentially an Arduino. Arduino code included in this repo.

%Format:  1     2        3        4        5       6       7
%         time, accel_x, accel_y, accel_z, gyro_x, gyro_y, gyro_z
"""
def parse():
    f = open('imu1_log.txt','r')
    fout = open('imu1_log_parsed.txt', 'w')
    times = list()
    for line in f:
        # grab time for stats on read quality and consistency
        split = line.split()
        if len(split) > 1:
            times.append(float(split[0]))
        outputline = re.sub('\s\$',',',line)
        fout.write(outputline)

    deltas = list()
    for i in range(len(times)-1):
        deltas.append(times[i+1] - times[i])
    print('Mean', statistics.mean(deltas))
    print('StdDev', statistics.stdev(deltas))
    print('Max', max(deltas))
    print('Min', min(deltas))
    f.close()
    fout.close()
    return times,deltas

def main():
    parse()

if __name__ == '__main__':
    main()