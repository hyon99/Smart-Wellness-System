**Smart Wellness System (STM32 + ESP32)**

A real-time embedded system for monitoring physiological and environmental parameters including ECG, Heart Rate, SpO₂, Body Temperature, and Ambient Conditions.

**Overview**

The Smart Wellness System is designed using a dual-microcontroller architecture:
STM32 → Data acquisition & OLED display
ESP32 → Signal processing & communication
The system provides real-time monitoring and visualization of multiple health parameters on both an OLED display and a PC.

**System Architecture**
<img width="1536" height="1024" alt="ChatGPT Image Mar 22, 2026, 09_04_22 PM" src="https://github.com/user-attachments/assets/1c9ca2b3-96eb-4780-88ea-aafde44f70dd" />



**Hardware Components**

STM32 Microcontroller
ESP32 Module
AD8232 ECG Sensor
MAX30102 Sensor
DHT11 Sensor
DS18B20 Sensor
SSD1306 OLED Display

**Software & Tools**
STM32CubeIDE
Arduino IDE
Python (Matplotlib)

**Features**

Real-time ECG waveform display
ECG-based heart rate detection
SpO₂ monitoring
Body temperature measurement
Ambient temperature & humidity sensing
Dual-microcontroller architecture
Serial/Bluetooth data transmission

**Results**

Stable ECG waveform obtained
Accurate heart rate from ECG
Real-time multi-parameter monitoring
Smooth data visualization

**Limitations**

Not medical-grade
Sensitive to motion artifacts
Requires proper sensor placement

**Future Scope**

IoT/cloud integration
Mobile application
AI-based analysis
Improved signal filtering
Portable version

**Authors**

Sudipon Makal
Tushar Ranjan Dash
Tushit Kumar
Soubhagya Mishra

**License**

For academic and research use only.
