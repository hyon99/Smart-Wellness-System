#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>

// ---------------- OLED ----------------
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// ---------------- AD8232 ----------------
#define ECG_PIN   PA0
#define LO_PLUS   PA1
#define LO_MINUS  PA2
#define SDN_PIN   PA3

// ---------------- DHT11 ----------------
#define DHTPIN PA4
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// ---------------- DS18B20 ----------------
#define DS_PIN PA7

// ---------------- Variables ----------------
int xPos = 0;
int prevY = 44;   // start mid graph
float temperature = 0;
float humidity = 0;
float bodyTemp = 0;

unsigned long lastDHTRead = 0;
unsigned long lastDSRead = 0;

// --------- ESP32 Data ----------
int heartRate = 0;
int spo2 = 0;
String serialBuffer = "";

// ================= DS18B20 FUNCTIONS =================
void DS_setOutput() { pinMode(DS_PIN, OUTPUT); }
void DS_setInput() { pinMode(DS_PIN, INPUT); }

bool DS_reset()
{
  DS_setOutput();
  digitalWrite(DS_PIN, LOW);
  delayMicroseconds(480);

  DS_setInput();
  delayMicroseconds(80);

  bool presence = !digitalRead(DS_PIN);
  delayMicroseconds(400);

  return presence;
}

void DS_writeBit(bool bit)
{
  DS_setOutput();
  digitalWrite(DS_PIN, LOW);
  delayMicroseconds(bit ? 1 : 60);

  DS_setInput();
  delayMicroseconds(bit ? 60 : 1);
}

void DS_writeByte(byte data)
{
  for (int i = 0; i < 8; i++)
  {
    DS_writeBit(data & 0x01);
    data >>= 1;
  }
}

byte DS_readByte()
{
  byte value = 0;
  for (int i = 0; i < 8; i++)
  {
    DS_setOutput();
    digitalWrite(DS_PIN, LOW);
    delayMicroseconds(2);

    DS_setInput();
    delayMicroseconds(8);

    if (digitalRead(DS_PIN))
      value |= (1 << i);

    delayMicroseconds(60);
  }
  return value;
}

float DS_getTemp()
{
  if (!DS_reset()) return -100;

  DS_writeByte(0xCC);
  DS_writeByte(0x44);
  delay(750);

  DS_reset();
  DS_writeByte(0xCC);
  DS_writeByte(0xBE);

  byte lowByte = DS_readByte();
  byte highByte = DS_readByte();

  int16_t temp = (highByte << 8) | lowByte;
  return temp / 16.0;
}

// ================= UART Parser =================
void readESP32()
{
  while (Serial1.available())
  {
    char c = Serial1.read();

    if (c == '>')
    {
      int hrIndex = serialBuffer.indexOf("HR:");
      int spIndex = serialBuffer.indexOf("SPO2:");

      if (hrIndex >= 0 && spIndex >= 0)
      {
        heartRate = serialBuffer.substring(
                      hrIndex + 3,
                      serialBuffer.indexOf(',', hrIndex)
                    ).toInt();

        spo2 = serialBuffer.substring(
                 spIndex + 5
               ).toInt();
      }
      serialBuffer = "";
    }
    else if (c != '<')
    {
      serialBuffer += c;
    }
  }
}

// ================= SETUP =================
void setup()
{
  pinMode(LO_PLUS, INPUT);
  pinMode(LO_MINUS, INPUT);
  pinMode(SDN_PIN, OUTPUT);
  digitalWrite(SDN_PIN, HIGH);

  dht.begin();
  Wire.begin();

  Serial.begin(115200);
  Serial1.begin(115200);

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.display();
}

// ================= LOOP =================
void loop()
{
  readESP32();

  if (millis() - lastDHTRead > 2000)
  {
    float t = dht.readTemperature();
    float h = dht.readHumidity();

    if (!isnan(t) && !isnan(h))
    {
      temperature = t;
      humidity = h;
    }
    lastDHTRead = millis();
  }

  if (millis() - lastDSRead > 1000)
  {
    float t = DS_getTemp();
    if (t > -50 && t < 100)
      bodyTemp = t;

    lastDSRead = millis();
  }

  float bodyTempF = (bodyTemp * 9.0 / 5.0) + 32.0;

  // -------- CLEAR FULL DISPLAY --------
  display.clearDisplay();

  // -------- TEXT AREA (0–25 px) --------
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(0, 0);
  display.print("T:");
  display.print(temperature, 1);
  display.print("C");

  display.setCursor(64, 0);
  display.print("H:");
  display.print(humidity, 0);
  display.print("%");

  display.setCursor(0, 10);
  display.print("Body:");
  display.print(bodyTempF, 1);
  display.print("F");

  display.setCursor(64, 10);
  display.print("HR:");
  display.print(heartRate);

  display.setCursor(64, 18);
  display.print("SpO2:");
  display.print(spo2);
  display.print("%");

  // -------- ECG GRAPH AREA (26–63 px) --------
  if (digitalRead(LO_PLUS) == 1 || digitalRead(LO_MINUS) == 1)
  {
    display.setCursor(30, 40);
    display.print("Leads Off!");
    display.display();
    return;
  }

  int ecgValue = analogRead(ECG_PIN);
  Serial1.println(ecgValue);
  Serial.println(ecgValue);

  int y = map(ecgValue, 0, 4095, 63, 26);

  display.drawLine(xPos - 1, prevY, xPos, y, SSD1306_WHITE);

  prevY = y;
  xPos++;

  if (xPos >= SCREEN_WIDTH)
  {
    xPos = 0;
    prevY = 44;
  }

  display.display();
  delay(4);
}