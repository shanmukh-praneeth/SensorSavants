#include <Adafruit_Sensor.h>
#include <Adafruit_ADXL345_U.h>

// IMU object
Adafruit_ADXL345_Unified accel = Adafruit_ADXL345_Unified(123);
float xOffset = 0, yOffset = 0, zOffset = 0;
float z=0,x=0,y=0;

#define RPWM 23
#define LPWM 19
#define REN 18
#define LEN 5
#define MOTOR_SPEED 200


void calibrateIMU() {
  float sumX = 0, sumY = 0, sumZ = 0;
  sensors_event_t event;
  for (int i = 0; i < 500; i++) {
    accel.getEvent(&event);
    sumX += event.acceleration.x;
    sumY += event.acceleration.y;
    sumZ += event.acceleration.z;
    delay(5);
  }
  xOffset = sumX / 500.0;
  yOffset = sumY / 500.0;
  zOffset = (sumZ / 500.0) - 9.81;
}

void openRoof() {
  if((z<8) && (z>0)){
    Serial.println("Opening roof...");
    analogWrite(RPWM, MOTOR_SPEED);
    analogWrite(LPWM, 0);
  }
  else{
    stopMotor();
  }
}

void closeRoof() {
  if((z>-4) && (z<3)){
    Serial.println("Closing roof...");
    analogWrite(RPWM, 0);
    analogWrite(LPWM, MOTOR_SPEED);
  }
  else{
    stopMotor();
  }
}

void stopMotor() {
  Serial.println("Stopping motor...");
  analogWrite(RPWM, 0);
  analogWrite(LPWM, 0);
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  if (!accel.begin()) {
    Serial.println("No ADXL345 detected!");
    while (1);
  }

  accel.setRange(ADXL345_RANGE_2_G);
  Serial.println("Calibrating IMU...");
  delay(2000);
  calibrateIMU();
  Serial.println("IMU calibration done.");

  digitalWrite(REN, HIGH);
  digitalWrite(LEN, HIGH);
}

void loop() {
  // put your main code here, to run repeatedly:
  // Get IMU data
  sensors_event_t event;
  accel.getEvent(&event);
  x = event.acceleration.x - xOffset;
  y = event.acceleration.y - yOffset;
  z = event.acceleration.z - zOffset;

  Serial.print("IMU → X: "); Serial.print(x);
  Serial.print(" Y: "); Serial.print(y);
  Serial.print(" Z: "); Serial.println(z);

  int isRaining = 1;

  // Detect if roof is open or closed (example logic)
  if (isRaining) {
    Serial.println("Rain detected & roof open → Closing roof!");
    closeRoof();
      // Add motor/servo control code to close roof
  }
  else if (!isRaining) {
    Serial.println("No rain & roof closed → Opening roof!");
    openRoof();
      // Add motor/servo control code to open roof
  }
  delay(500);
}