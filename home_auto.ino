#include <Wire.h>
#include <Adafruit_BMP085.h>
#include <DHT.h>

HardwareSerial dwin(2);

// ================= SAFE GPIO =================
// Bedroom
#define BR1 13
#define BR2 14
#define BR3 27
#define BR4 26

// Living
#define LR1 25
#define LR2 33
#define LR3 32
#define LR4 23

// ================= DHT =================
#define DHTPIN 4
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// ================= BMP =================
Adafruit_BMP085 bmp;

// ================= TIMER =================
unsigned long lastSensorUpdate = 0;
const unsigned long sensorInterval = 2000;

// =========================================
void sendValue(uint16_t vp, uint16_t value)
{
  byte frame[8] = {
    0x5A, 0xA5, 0x05, 0x82,
    highByte(vp), lowByte(vp),
    highByte(value), lowByte(value)
  };
  dwin.write(frame, 8);
}

// =========================================
void handleLED(uint16_t vp, uint16_t value)
{
  bool state = (value == 1);

  switch (vp)
  {
    case 0x8000: digitalWrite(BR1, state); break;
    case 0x8100: digitalWrite(BR2, state); break;
    case 0x8200: digitalWrite(BR3, state); break;
    case 0x8300: digitalWrite(BR4, state); break;

    case 0x9000: digitalWrite(LR1, state); break;
    case 0x9100: digitalWrite(LR2, state); break;
    case 0x9200: digitalWrite(LR3, state); break;
    case 0x9300: digitalWrite(LR4, state); break;
  }
}

// =========================================
void readDWIN()
{
  while (dwin.available() >= 8)
  {
    if (dwin.read() == 0x5A && dwin.read() == 0xA5)
    {
      dwin.read();                 // length
      byte cmd = dwin.read();      // command

      if (cmd == 0x83)
      {
        byte vpH = dwin.read();
        byte vpL = dwin.read();
        byte dataH = dwin.read();
        byte dataL = dwin.read();

        uint16_t vp = (vpH << 8) | vpL;
        uint16_t value = (dataH << 8) | dataL;

        handleLED(vp, value);
      }
    }
  }
}

// =========================================
void setup()
{
  Serial.begin(115200);
  dwin.begin(115200, SERIAL_8N1, 16, 17);

  int pins[] = {BR1,BR2,BR3,BR4,LR1,LR2,LR3,LR4};

  for(int i=0;i<8;i++)
  {
    pinMode(pins[i], OUTPUT);
    digitalWrite(pins[i], LOW);
  }

  dht.begin();

  if (!bmp.begin())
  {
    Serial.println("BMP180 not detected");
    while(1);
  }
}

// =========================================
void loop()
{
  readDWIN();

  // Non-blocking sensor update
  if (millis() - lastSensorUpdate > sensorInterval)
  {
    lastSensorUpdate = millis();

    float h = dht.readHumidity();
    float t = dht.readTemperature();
    float p = bmp.readPressure() / 100.0;

    if (!isnan(h) && !isnan(t))
    {
      sendValue(0x6000, t * 10);   // Temp (1 decimal)
      sendValue(0x6100, h);        // Humidity
      sendValue(0x6200, p);        // Pressure
    }
  }
}