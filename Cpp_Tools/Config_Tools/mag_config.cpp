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
    
    // Ensure orientation is selected, options are LEVEL, SIDEWAYS, and FLATFRONT (enumerated in HMC6343 header)
    compass.setOrientation(SIDEWAYS);

    // Enter user calibration mode
    printf("Entering calibration mode. Please rotate the vehicle and/or sensor.\n");
    compass.enterCalMode();
    while(!done) {
        // Wait for minimum time
        bcm2835_delay(200);
    }
    printf("Exiting calibration mode.\n");
    compass.exitCalMode();
    compass.reset();
    printf("Calibration complete. Axis offset values will be saved to the EEPROM automatically.\n");
    return 0;
}

