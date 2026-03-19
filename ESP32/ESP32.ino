#include <Wire.h>
#include "MAX30105.h"
#include "heartRate.h"

MAX30105 particleSensor;

// UART2 → Used for BOTH sending to STM32 and receiving ECG
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
  Serial.begin(115200);  // USB → Laptop

  // 🔹 USE UART2 (RX2=GPIO16, TX2=GPIO17)
  uart2.begin(115200, SERIAL_8N1, 16, 17);

  Wire.begin(21, 22);

  if (!particleSensor.begin(Wire, I2C_SPEED_FAST))
  {
    Serial.println("MAX30102 not found");
    while (1);
  }

  particleSensor.setup();
  particleSensor.setPulseAmplitudeRed(0x1F);
  particleSensor.setPulseAmplitudeIR(0x1F);

  Serial.println("System Ready");
}

void loop()
{
  // -------- RECEIVE ECG FROM STM32 --------
  while (uart2.available())
  {
    Serial.write(uart2.read());   // Direct forward to USB
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

  // -------- SEND SPO2 + HR TO STM32 --------
  if (millis() - lastSend > 1000)
  {
    lastSend = millis();

    uart2.print("<HR:");
    uart2.print(beatAvg);
    uart2.print(",SPO2:");
    uart2.print(spo2);
    uart2.println(">");
  }
}
