
#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_DRV2605.h>

// Create sensor objects
Adafruit_MPU6050 mpu1; // Address 0x68 (AD0 -> GND)
Adafruit_MPU6050 mpu2; // Address 0x69 (AD0 -> VCC)

// Haptic driver
Adafruit_DRV2605 drv;

// Simple motion threshold for demo (degrees/sec)
const float GYRO_Y_THRESHOLD = 200.0;
unsigned long lastHaptic = 0;
const unsigned long hapticCooldown = 500; // milliseconds between vibrations

void setup() {
  Serial.begin(9600);
  delay(200);
  Serial.println("BOOTED");
  Wire.begin(21, 22); // SDA, SCL on ESP32

  // Initialize MPU6050 #1 at 0x68
  if (!mpu1.begin(0x68, &Wire)) {
    Serial.println("MPU6050 #1 not found at 0x68!");
    while (1) { delay(10); }
  }

  // Initialize MPU6050 #2 at 0x69
  if (!mpu2.begin(0x69, &Wire)) {
    Serial.println("MPU6050 #2 not found at 0x69!");
    while (1) { delay(10); }
  }

  // Configure accelerometer and gyro ranges
  mpu1.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu1.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu1.setFilterBandwidth(MPU6050_BAND_21_HZ);

  mpu2.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu2.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu2.setFilterBandwidth(MPU6050_BAND_21_HZ);

  // Initialize DRV2605 haptic driver (default address 0x5A)
  if (!drv.begin()) {
    Serial.println("DRV2605 not found at 0x5A!");
    while (1) { delay(10); }
  }
  drv.selectLibrary(1);                 // choose haptic library
  drv.setMode(DRV2605_MODE_INTTRIG);    // internal trigger mode

  // Preload a simple click effect for testing
  drv.setWaveform(0, 1); // effect ID 1
  drv.setWaveform(1, 0); // end
  drv.go();

  delay(200);
  Serial.println("Setup complete!");
}

void loop() {
  sensors_event_t a1, g1, temp1;
  sensors_event_t a2, g2, temp2;

  // Read both sensors
  mpu1.getEvent(&a1, &g1, &temp1);
  mpu2.getEvent(&a2, &g2, &temp2);

  // Print accelerometer readings
  Serial.print("MPU1 Accel: ");
  Serial.print(a1.acceleration.x); Serial.print(", ");
  Serial.print(a1.acceleration.y); Serial.print(", ");
  Serial.println(a1.acceleration.z);

  Serial.print("MPU2 Accel: ");
  Serial.print(a2.acceleration.x); Serial.print(", ");
  Serial.print(a2.acceleration.y); Serial.print(", ");
  Serial.println(a2.acceleration.z);

  // Print gyroscope readings
  Serial.print("MPU1 Gyro: ");
  Serial.print(g1.gyro.x); Serial.print(", ");
  Serial.print(g1.gyro.y); Serial.print(", ");
  Serial.println(g1.gyro.z);

  Serial.print("MPU2 Gyro: ");
  Serial.print(g2.gyro.x); Serial.print(", ");
  Serial.print(g2.gyro.y); Serial.print(", ");
  Serial.println(g2.gyro.z);

  // Simple demo: trigger haptic if MPU1 gyro Y exceeds threshold
  float degsY = g1.gyro.y * 57.2958f; // rad/s → deg/s
  if ((millis() - lastHaptic) > hapticCooldown && degsY > GYRO_Y_THRESHOLD) {
    Serial.println("Motion detected -> Vibrate!");
    drv.setWaveform(0, 11); // strong buzz
    drv.setWaveform(1, 0);  // end
    drv.go();
    lastHaptic = millis();
  }

  delay(20); // ~50 Hz loop
}