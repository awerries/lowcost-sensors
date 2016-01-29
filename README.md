# R-Pi Sensors
A set of software tools for collecting sensor data on the Raspberry Pi. I use these for my own research, which is published at https://github.com/awerries/kalman-localization. 

The Cpp_Tools folder contains drivers for RPi I2C, the HMC6343 magnetometer, and the MPU-6050 inertial measurement unit, along with programs for logging data from each one. The driver code itself is borrowed from elsewhere, but I wrote the Readers and Config_Tools.

The Python_Tools folder contains scripts for reading serial data, which I created specifically to timestamp and save data from a Sparkfun Razor IMU and a NovAtel GPS. It also contains a script for performing frequency analysis of arbitrary columns of data from any CSV-file.

The RazorIMU folder is the on-board Arduino firmware that I use for the Sparkfun Razor IMU https://www.sparkfun.com/products/10736, subsequently parsed by the Python_Tools *read\_imu\_log.py* code. I modified the Arduino code from Peter Bartz's AHRS code at https://github.com/ptrbrtz/razor-9dof-ahrs.
