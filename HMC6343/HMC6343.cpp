/******************************************************************************
HMC6343.cpp
Core implementation file for the HMC6343 3-axis compass library.
Jordan McConnell @ SparkFun Electronics
17 July 2014
https://github.com/sparkfun/HMC6343_Breakout

This file implements the functions of the HMC6343 sensor class as well as
providing documentation on what each library function does.

Developed/Tested with:
Arduino Uno
Arduino IDE 1.0.5 & 1.5.2

This code is beerware; if you see me (or any other SparkFun employee) at the 
local, and you've found our code helpful, please buy us a round!
Distributed as-is; no warranty is given. 
******************************************************************************/

#include "HMC6343.h"

// Constructor - Creates sensor object, sets I2C address, and initializes sensor variables
HMC6343::HMC6343() {
    _addr = HMC6343_I2C_ADDR;
    
    heading = pitch = roll = 0;
    magX = magY = magZ = 0;
    accelX = accelY = accelZ = 0;
    temperature = 0;
    
    clearRawData();
}

// Destructor - Deletes sensor object
HMC6343::~HMC6343() {

}

// Initialize - returns true if successful
// Starts I2C Communication
// Verifies sensor is there by checking its I2C Address in its EEPROM
bool HMC6343::init() {
    bool ret = true;
    uint8_t data = 0x00;
    
    // Check for device by reading I2C address from EEPROM
    data = readEEPROM(SLAVE_ADDR);
    
    // Check if value read in EEPROM is the expected value for the HMC6343 I2C address
    if (!(data == 0x32)) {
      ret = false; // Init failed, EEPROM did not read expected I2C address value
    }
      
    return ret;
}

// Send the HMC6343 a command to read the raw magnetometer values
// Store these values in the integers magX, magY, and magZ
void HMC6343::readMag() {
    readGeneric(POST_MAG, &magX, &magY, &magZ);
}

// Send the HMC6343 a command to read the raw accelerometer values
// Store these values in the integers accelX, accelY, and accelZ
void HMC6343::readAccel() {
    readGeneric(POST_ACCEL, &accelX, &accelY, &accelZ);
}

// Send the HMC6343 a command to read the raw calculated heading values
// Store these values in the integers heading, pitch, and roll
void HMC6343::readHeading() {
    readGeneric(POST_HEADING, &heading, &pitch, &roll);
}

// Send the HMC6343 a command to read the raw calculated tilt values
// Store these values in the integers pitch, roll, and temperature
void HMC6343::readTilt() {
    readGeneric(POST_TILT, &pitch, &roll, &temperature);
}

// Generic function which sends the HMC6343 a specified command
// It then collects 6 bytes of data via I2C and consolidates it into three integers
// whose addresses are passed to the function by the above read commands
void HMC6343::readGeneric(uint8_t command, int16_t* first, int16_t* second, int16_t* third) {
    sendCommand(command); // Send specified I2C command to HMC6343
    bcm2835_delay(1); // Delay response time
    
    clearRawData(); // Clear object's rawData[] array before storing new values in the array
    
    // Read 6 byte response via I2C and store them in rawData[] array
    I2Cdev::readBytes(_addr, 6, rawData);
    
    // Convert 6 bytes received into 3 integers
    *first = rawData[0] << 8; // MSB
    *first |= rawData[1];     // LSB
    *second = rawData[2] << 8;
    *second |= rawData[3];
    *third = rawData[4] << 8;
    *third |= rawData[5];
}

// Send specified I2C command to HMC6343
void HMC6343::sendCommand(uint8_t command) {
    I2Cdev::writeByte(_addr, command);
}

// Send enter standby mode I2C command to HMC6343
void HMC6343::enterStandby() {
    sendCommand(ENTER_STANDBY);
}

// Send exit standby (enter run) mode I2C command to HMC6343
void HMC6343::exitStandby() {
    sendCommand(ENTER_RUN);
}

// Send enter sleep mode I2C command to HMC6343
void HMC6343::enterSleep() {
    sendCommand(ENTER_SLEEP);
}

// Send exit sleep mode I2C command to HMC6343
void HMC6343::exitSleep() {
    sendCommand(EXIT_SLEEP);
}

// Send enter calibration mode I2C command to HMC6343
void HMC6343::enterCalMode() {
    sendCommand(ENTER_CAL);
}

// Send exit calibration mode I2C command to HMC6343
void HMC6343::exitCalMode() {
    sendCommand(EXIT_CAL);
}

// Set the physical orientation of the HMC6343 IC to either LEVEL, SIDEWAYS, or FLATFRONT
// This allows the IC to calculate a proper heading, pitch, and roll in tenths of degrees
// LEVEL      X = forward, +Z = up (default)
// SIDEWAYS   X = forward, +Y = up
// FLATFRONT  Z = forward, -X = up
void HMC6343::setOrientation(uint8_t orientation) {
    if (orientation == LEVEL) {
      sendCommand(ORIENT_LEVEL);
    }
    else if (orientation == SIDEWAYS) {
      sendCommand(ORIENT_SIDEWAYS);
    }
    else if (orientation == FLATFRONT) {
      sendCommand(ORIENT_FLATFRONT);
    }
}

// Send the I2C command to reset the processor on the HMC6343
void HMC6343::reset() {
    sendCommand(RESET);
}

// Send the I2C command to read the OPMode1 status register of the HMC6343
// The register informs you of current calculation status, filter status, modes enabled and what orientation
// Refer to the HMC6343 datasheet for bit specifics
uint8_t HMC6343::readOPMode1() {
    uint8_t opmode1 = 0x00;
    sendCommand(POST_OPMODE1);
    bcm2835_delay(1);
    I2Cdev::readBytes(_addr, 1, &opmode1);
    return opmode1;
}

// Send a command to the HMC6343 to read a specified register of the EEPROM
uint8_t HMC6343::readEEPROM(uint8_t reg) {
    uint8_t data = 0x00;
    
    I2Cdev::writeByte(_addr, READ_EEPROM, reg);
    bcm2835_delay(10);
    I2Cdev::readBytes(_addr, 1, &data);
    
    return data;
}

// Send a command to the HMC6343 to write a specified register of the EEPROM
void HMC6343::writeEEPROM(uint8_t reg, uint8_t data) {
    uint8_t out[] = {reg, data};
    I2Cdev::writeBytes(_addr, WRITE_EEPROM, 2, out);
}

// Clears the sensor object's rawData[] array, used before taking new measurements
void HMC6343::clearRawData() {
    for (uint8_t i = 0; i < 6; i++) {
      rawData[i] = 0;
    }
}
