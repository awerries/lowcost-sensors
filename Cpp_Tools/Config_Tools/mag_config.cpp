// Standard libraries
#include <stdio.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>

// Libraries for I2C and the HMC6343 sensor
#include <bcm2835.h>
#include "HMC6343.h"

#define PI 3.14159265359

// Signal handler callback function
volatile sig_atomic_t done = 0;
void sig_handler(int signum) {
    done = 1;
}

// Helper function for grabbing all offset values
void getOffsets(HMC6343* compass, int16_t* xoffset, int16_t* yoffset, int16_t* zoffset) {
    *xoffset = compass->readEEPROM(XOFFSET_MSB) << 8;
    *xoffset |= compass->readEEPROM(XOFFSET_LSB);
    *yoffset = compass->readEEPROM(YOFFSET_MSB) << 8;
    *yoffset |= compass->readEEPROM(XOFFSET_LSB);
    *zoffset = compass->readEEPROM(ZOFFSET_MSB) << 8;
    *zoffset |= compass->readEEPROM(ZOFFSET_LSB);
}

int main() {
    // Set up signal handler
    struct sigaction action;
    memset(&action, 0, sizeof(struct sigaction));
    action.sa_handler = sig_handler;
    sigaction(SIGTERM, &action, NULL);
    sigaction(SIGINT, &action, NULL);

    // Create new file with timestamp
    char filename_buffer[255];
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    sprintf(filename_buffer, "mag_data_%04d-%02d-%02dT%02d%02d%02d.log", 
            tm.tm_year+1900, tm.tm_mon+1, tm.tm_mday, 
            tm.tm_hour, tm.tm_min, tm.tm_sec);
    FILE *f = fopen(filename_buffer, "w");

    // Initialize I2C and the compass itself
    I2Cdev::initialize();
    HMC6343 compass;
    if (!compass.init()) {
      // Report failure, is the sensor wiring correct?
      printf("Sensor Initialization Failed\n");
    }
    
    // Ensure orientation is selected, options are LEVEL, SIDEWAYS, and FLATFRONT (enumerated in HMC6343 header)
    compass.setOrientation(SIDEWAYS);

    // Initialize time
    struct timeval start_time, current_time;

    // Print calibration values
    int16_t xoffset, yoffset, zoffset;
    bcm2835_delay(1000);
    getOffsets(&compass, &xoffset, &yoffset, &zoffset);
    printf("Prior offset values: %d, %d, %d\n", xoffset, yoffset, zoffset);

    // Enter user calibration mode
    printf("Entering calibration mode. Please rotate the vehicle and/or sensor.\n");
    compass.enterCalMode();
    while(!done) {
        // Read compass data
        gettimeofday(&start_time, NULL);
        compass.readTilt();
        compass.readHeading();
        compass.readAccel();
        compass.readMag();
        gettimeofday(&current_time, NULL);
        // Print start and end times of measurement
        fprintf(f,"%ld.%06ld,%ld.%06ld,",
            (long int) start_time.tv_sec, (long int) start_time.tv_usec, 
            (long int) current_time.tv_sec, (long int) current_time.tv_usec);
        // Print yaw, pitch, roll in radians
        fprintf(f,"%0.6f,%0.6f,%0.6f,",
                (float) compass.heading/10.0*PI/180.0, (float) compass.pitch/10.0*PI/180.0, (float) compass.roll/10.0*PI/180.0);
        // Print accel xyz in g's
        fprintf(f,"%0.6f,%0.6f,%0.6f,",
                (float) compass.accelX/1024.0, (float) compass.accelY/1024.0, (float) compass.accelZ/1024.0);
        // Print temperature in Celsius?
        fprintf(f,"%0.4f\n", (float) compass.temperature);
        fflush(f);
        
        // Wait for minimum time
        bcm2835_delay(200);
    }
    printf("Exiting calibration mode.\n");
    compass.exitCalMode();
    bcm2835_delay(1000);
    compass.reset();
    bcm2835_delay(1000);
    // Print calibration values
    getOffsets(&compass, &xoffset, &yoffset, &zoffset);
    printf("Final offset values: %d, %d, %d\n", xoffset, yoffset, zoffset);

    fclose(f);
    printf("Calibration complete. Axis offset values will be saved to the EEPROM automatically.\n");
    return 0;
}

