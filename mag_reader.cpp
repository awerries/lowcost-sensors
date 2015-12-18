// Libraries for I2C and the HMC6343 sensor
#include "HMC6343/HMC6343.h"
#include <bcm2835.h>
#include <stdio.h>

void printHeadingData(HMC6343 compass);
void printAccelData(HMC6343 compass);

int main() {
    I2Cdev::initialize();
    HMC6343 compass; // Declare the sensor object
    if (!compass.init()) {
      printf("Sensor Initialization Failed\n"); // Report failure, is the sensor wiring correct?
    }

    while(true) {
        // Read, calculate, and print the heading, pitch, and roll from the sensor
        compass.readHeading();
        printHeadingData(compass);
        
        // Read, calculate, and print the acceleration on the x, y, and z axis of the sensor
        compass.readAccel();
        printAccelData(compass);
        
        // Wait for minimum time
        bcm2835_delay(200);
    }
    return 0;
}

// Print both the raw values of the compass heading, pitch, and roll
// as well as calculate and print the compass values in degrees
// Sample Output:
// Heading Data (Raw value, in degrees):
// Heading: 3249  324.90°
// Pitch:   28    2.80°
// Roll:    24    2.40°
void printHeadingData(HMC6343 compass) {   
    printf("Heading Data (Raw value, in degrees):\n");
    printf("  Heading: %04d, %0.4f\u00b0\n",compass.heading, (float) compass.heading/10.0);
    printf("  Pitch  : %04d, %0.4f\u00b0\n",compass.pitch, (float) compass.pitch/10.0);
    printf("  Roll   : %04d, %0.4f\u00b0\n",compass.roll, (float) compass.roll/10.0);
}

// Print both the raw values of the compass acceleration measured on each axis
// as well as calculate and print the accelerations in g forces
// Sample Output:
// Accelerometer Data (Raw value, in g forces):
// X: -52    -0.05g
// Y: -44    -0.04g
// Z: -1047  -1.02g
void printAccelData(HMC6343 compass) {
    printf("Accelerometer Data (Raw value, in g forces):\n");
    printf("  X: %04d, %0.4fg\n", compass.accelX, (float) compass.accelX/1024.0);
    printf("  Y: %04d, %0.4fg\n", compass.accelY, (float) compass.accelY/1024.0);
    printf("  Z: %04d, %0.4fg\n", compass.accelZ, (float) compass.accelZ/1024.0);
    printf("\n");
}

