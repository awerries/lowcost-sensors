#include <Wire.h>
// Arduino backward compatibility macros
#if ARDUINO >= 100
  #define WIRE_SEND(b) Wire.write((byte) b) 
  #define WIRE_RECEIVE() Wire.read() 
#else
  #define WIRE_SEND(b) Wire.send(b)
  #define WIRE_RECEIVE() Wire.receive() 
#endif

#define STATUS_LED_PIN 13  // Pin number of status LED

int accel[3]; // Actually stores the NEGATED acceleration (equals gravity, if board not moving).
int gyro[3];
int mag[3];
bool output_errors = false;

void I2C_Init() {
  Wire.begin();
}

void setup() {
  // Init serial output
  Serial.begin(115200);
  // Init status LED
  pinMode (STATUS_LED_PIN, OUTPUT);
  digitalWrite(STATUS_LED_PIN, HIGH);
  // Init sensors
  delay(50);  // Give sensors enough time to start
  I2C_Init();
  Accel_Init();
  Magn_Init();
  Gyro_Init();
}

void loop() {
  // Output data at 400Hz if possible!
  Read_Gyro(); // Read gyroscope
  Read_Accel(); // Read accelerometer
  Serial.print('$');
  Serial.print(accel[0]); Serial.print(",");
  Serial.print(accel[1]); Serial.print(",");
  Serial.print(accel[2]); Serial.print(",");
  Serial.print(gyro[0]); Serial.print(",");
  Serial.print(gyro[1]); Serial.print(",");
  Serial.print(gyro[2]);
  Serial.println();
  Serial.flush();
  delay(5);
}
