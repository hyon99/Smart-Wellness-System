#include <Wire.h>
#include "MAX30105.h"
#include "heartRate.h"
#include "BluetoothSerial.h"

MAX30105 particleSensor;
BluetoothSerial SerialBT;

// UART2 → STM32
HardwareSerial uart2(2);

// Heart rate variables
const byte RATE_SIZE = 4;
byte rates[RATE_SIZE];
byte rateSpot = 0;
long lastBeat = 0;

float beatsPerMinute;
int beatAvg = 0;
int spo2 = 0;

unsigned long lastSend = 0;

void setup()
{
  Serial.begin(115200);          // USB (optional debug)
  SerialBT.begin("ESP32_ECG");   // 🔵 Bluetooth name

  uart2.begin(115200, SERIAL_8N1, 16, 17); // RX2, TX2

  Wire.begin(21, 22);

  if (!particleSensor.begin(Wire, I2C_SPEED_FAST))
  {
    Serial.println("MAX30102 not found");
    SerialBT.println("MAX30102 not found");
    while (1);
  }

  particleSensor.setup();
  particleSensor.setPulseAmplitudeRed(0x1F);
  particleSensor.setPulseAmplitudeIR(0x1F);

  Serial.println("System Ready");
  SerialBT.println("Bluetooth Connected - ESP32 Ready");
}

void loop()
{
  // -------- RECEIVE ECG FROM STM32 --------
  while (uart2.available())
  {
    char c = uart2.read();

    Serial.write(c);      // USB
    SerialBT.write(c);    // 🔵 Bluetooth
  }

  // -------- SPO2 + HR PROCESSING --------
  long irValue = particleSensor.getIR();

  if (irValue < 50000)
  {
    beatAvg = 0;
    spo2 = 0;
  }
  else
  {
    if (checkForBeat(irValue))
    {
      long delta = millis() - lastBeat;
      lastBeat = millis();

      beatsPerMinute = 60 / (delta / 1000.0);

      if (beatsPerMinute > 20 && beatsPerMinute < 255)
      {
        rates[rateSpot++] = (byte)beatsPerMinute;
        rateSpot %= RATE_SIZE;

        beatAvg = 0;
        for (byte i = 0; i < RATE_SIZE; i++)
          beatAvg += rates[i];

        beatAvg /= RATE_SIZE;
      }
    }

    long redValue = particleSensor.getRed();
    float ratio = (float)redValue / (float)irValue;
    spo2 = 110 - (25 * ratio);

    if (spo2 > 100) spo2 = 100;
    if (spo2 < 0) spo2 = 0;
  }

  // -------- SEND SPO2 + HR --------
  if (millis() - lastSend > 1000)
  {
    lastSend = millis();

    String data = "<HR:" + String(beatAvg) + ",SPO2:" + String(spo2) + ">";

    uart2.println(data);     // STM32
    Serial.println(data);    // USB
    SerialBT.println(data);  // 🔵 Bluetooth
  }
}
