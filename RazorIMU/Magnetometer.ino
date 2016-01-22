#define MAGN_ADDRESS  ((int) 0x1E) // 0x1E = 0x3C / 2
// http://cdn.sparkfun.com/datasheets/Sensors/Magneto/HMC5883L-FDS.pdf

void Magn_Init() {
  Wire.beginTransmission(MAGN_ADDRESS);
  WIRE_SEND(0x02); 
  WIRE_SEND(0x00);  // Set continuous mode (default 10Hz)
  Wire.endTransmission();
  delay(5);

  Wire.beginTransmission(MAGN_ADDRESS);
  WIRE_SEND(0x00);
  WIRE_SEND(0b00011000);  // Set 50Hz
  Wire.endTransmission();
  delay(5);
}

void Read_Magn() {
  int i = 0;
  byte buff[6];
 
  Wire.beginTransmission(MAGN_ADDRESS); 
  WIRE_SEND(0x03);  // Send address to read from
  Wire.endTransmission();
  
  Wire.beginTransmission(MAGN_ADDRESS); 
  Wire.requestFrom(MAGN_ADDRESS, 6);  // Request 6 bytes
  while(Wire.available()) {  // ((Wire.available())&&(i<6))
    buff[i] = WIRE_RECEIVE();  // Read one byte
    i++;
  }
  Wire.endTransmission();
  
  if (i == 6) { // All bytes received?
    // MSB byte first, then LSB; Y and Z reversed: X, Z, Y
    mag[0] = -1 * ((((int) buff[4]) << 8) | buff[5]);  // X axis (internal sensor -y axis)
    mag[1] = -1 * ((((int) buff[0]) << 8) | buff[1]);  // Y axis (internal sensor -x axis)
    mag[2] = -1 * ((((int) buff[2]) << 8) | buff[3]);  // Z axis (internal sensor -z axis)
  } else if (output_errors) {
    Serial.println("!ERR: reading magnetometer");
  }
}
