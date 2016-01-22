#!/usr/bin/env python
""" Code parses BESTXYZ output from NovAtel GPS and writes to a matlab-friendly csv file.

Author: Adam Werries, awerries@cmu.edu

For pre-parsed format, see pg420 at http://www.novatel.com/assets/Documents/Manuals/om-20000129.pdf 

New Format:  1     2                   3               4  5  6  7      8      9      10  11  12  13      14      15      16        17
             time, sol good? (1 or 0), WAAS? (1 or 0), x, y, z, sig_x, sig_y, sig_z, vx, vy, vz, sig_vx, sig_vy, sig_vz, num_sats, sol_sats 
"""
import statistics
import re
def parse():
	f = open('gps_log.txt','r')
	fout = open('gps_log_parsed.txt', 'w')
	times = list()
	for line in f:
		# grab time for stats on read quality and consistency
		outputline = list()
		timeAndPacket = line.split()
		if len(timeAndPacket) > 1:
			time = timeAndPacket[0]
			times.append(float(time))
			outputline.append(time)
			headerAndData = timeAndPacket[1].split(';')
			dataFields = headerAndData[1].split(',')
			
			# Check solution quality, output 1 if good, 0 if meh, skip if terrible
			if dataFields[0] == 'SOL_COMPUTED':
				outputline.append('1')
			elif dataFields[0] == 'INSUFFICIENT_OBS':
				continue
			else:
				outputline.append('0')
			if dataFields[1] == 'WAAS':
				outputline.append('1')
			else:
				outputline.append('0')
			# Add position and position-stdev info
			outputline.extend(dataFields[2:8])
			# Add velocity and velocity-stdev info
			outputline.extend(dataFields[10:16])
			# Add velocity-calculation latency
			outputline.append(dataFields[17])
			# Add satellites tracked and used in solution
			outputline.extend(dataFields[20:22])
			# Output to new file
			fout.write(','.join(outputline)+'\n')

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