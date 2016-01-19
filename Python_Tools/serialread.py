#!/usr/bin/env python
"""Program written to read from serial ports for the purpose of logging timestamped data from serial sensors.
Current implementation simply logs from three serial ports hardcoded into the program, which can be
enabled/disabled based on setting the port_flags"""
import sys
import serial
import io
from time import sleep, time
from datetime import datetime

start = b'4'

def open_serial(port_flags):
    imu1, imu2, gps = None, None, None
    if port_flags[0]:
        imu1 = serial.Serial(port='/dev/ttyUSB0', baudrate=57600,
                parity=serial.PARITY_NONE, stopbits=serial.STOPBITS_ONE, 
                bytesize=serial.EIGHTBITS, timeout=0)

    if port_flags[1]:
        imu2 = serial.Serial(port='/dev/ttyUSB1', baudrate=115200,
                parity=serial.PARITY_NONE, stopbits=serial.STOPBITS_ONE,
                bytesize=serial.EIGHTBITS, timeout=0)

    if port_flags[2]:
        gps = serial.Serial(port='/dev/ttyUSB0', baudrate=115200,
                parity=serial.PARITY_NONE, stopbits=serial.STOPBITS_ONE,
                bytesize=serial.EIGHTBITS, timeout=0)

    return imu1,imu2,gps

def log_imu(ser, outputfile, line, hasStart):
    output = ser.read().decode('utf-8')
    if output == '$':
        line = [output]
        hasStart = True
    elif hasStart:
        line.append(output)
        if output == '\n':
            save_output(outputfile, ''.join(line))
            hasStart = False
    return line, hasStart

def log_gps(ser, outputfile, line, hasStart):
    output = ser.read().decode('utf-8')
    if output == '#':
        line = [output]
        hasStart = True
    elif hasStart:
        line.append(output)
        if output == '\n':
            save_output(outputfile, ''.join(line))
            hasStart = False
    return line, hasStart

def save_output(outputfile, output):
    if output:
        outputfile.write(str(time()))
        outputfile.write(' ')
        outputfile.write(output)
        print(output,end='')

def main(argv):
    port_flags = [int(argv[1]), int(argv[2]), int(argv[3])]
    imu1, imu2, gps = None, None, None
    imu1_log, imu2_log, gps_log = None, None, None
    imu1_start, imu2_start, gps_start = False, False, False
    line1 = list()
    line2 = list()
    line3 = list()

    try:
        print('Opening serial...')
        (imu1, imu2, gps) = open_serial(port_flags)
        sleep(2)
        if imu1:
            imu1_log = open('imu1_log.txt', 'w')
            imu1.readline()
            imu1.write(start)
            imu1.flush()
        if imu2:
            imu2_log = open('imu2_log.txt', 'w')
            imu2.flush()
        if gps:
            gps_log = open('gps_log.txt', 'w')
            gps.readline()

        print('Starting logging...')
        while True:
            if imu1:
                line1,imu1_start = log_imu(imu1, imu1_log, line1, imu1_start)
            if imu2:
                line2,imu2_start = log_imu(imu2, imu2_log, line2, imu2_start)
            if gps:
                line3,gps_start = log_gps(gps, gps_log, line3, gps_start)

    finally:
        if imu1:
            imu1.close()
            imu1_log.close()
        if imu2:
            imu2.close()
            imu2_log.close()
        if gps:
            gps.close()
            gps_log.close()

if __name__=='__main__':
    main(sys.argv)
