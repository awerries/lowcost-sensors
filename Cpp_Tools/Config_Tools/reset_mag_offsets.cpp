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

    // Initialize I2C and the compass itself
    I2Cdev::initialize();
    HMC6343 compass;
    if (!compass.init()) {
      // Report failure, is the sensor wiring correct?
      printf("Sensor Initialization Failed\n");
    }

    // Initialize time
    struct timeval start_time, current_time;

    // Print calibration values
    int16_t xoffset, yoffset, zoffset;
    bcm2835_delay(1000);
    getOffsets(&compass, &xoffset, &yoffset, &zoffset);
    printf("Prior offset values: %d, %d, %d\n", xoffset, yoffset, zoffset);

    compass.writeEEPROM(XOFFSET_MSB, 0); bcm2835_delay(200);
    compass.writeEEPROM(XOFFSET_LSB, 0); bcm2835_delay(200);
    compass.writeEEPROM(YOFFSET_MSB, 0); bcm2835_delay(200);
    compass.writeEEPROM(YOFFSET_LSB, 0); bcm2835_delay(200);
    compass.writeEEPROM(ZOFFSET_MSB, 0); bcm2835_delay(200);
    compass.writeEEPROM(ZOFFSET_LSB, 0); bcm2835_delay(200);

    // Print calibration values
    getOffsets(&compass, &xoffset, &yoffset, &zoffset);
    printf("Final offset values: %d, %d, %d\n", xoffset, yoffset, zoffset);
    compass.reset();
    return 0;
}

