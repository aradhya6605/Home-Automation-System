#define BLYNK_TEMPLATE_ID "TMPL30-da-19o"
#define BLYNK_TEMPLATE_NAME "Home Automation"
#define BLYNK_AUTH_TOKEN "dxR4sznAvidAO2QFaKedHrwdNDI_iipN"

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <Wire.h>
#include <Adafruit_BMP085.h>
#include <DHT.h>

// ================= WIFI =================
char ssid[] = "    ";
char pass[] = "   ";

// ================= DWIN =================
HardwareSerial dwin(2);  // UART2 RX=16 TX=17
byte Buffer[9];

// ================= RELAYS =================
#define R1 13
#define R2 12
#define R3 14
#define R4 27
#define R5 26
#define R6 25
#define R7 33
#define R8 32
#define R9 23
#define R10 19

// ================= PIR =================
#define PIR_PIN 18
unsigned long lastMotionTime = 0;
const unsigned long motionDelay = 300000;
bool lightState = false;

// ================= DHT =================
#define DHTPIN 4
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// ================= BMP180 =================
Adafruit_BMP085 bmp;

BlynkTimer timer;

// =================================================
// ================= BLYNK RELAY CONTROL ==========
// =================================================
BLYNK_WRITE(V0){ setRelay(R1, param.asInt()); }
BLYNK_WRITE(V1){ setRelay(R2, param.asInt()); }
BLYNK_WRITE(V2){ setRelay(R3, param.asInt()); }
BLYNK_WRITE(V3){ setRelay(R4, param.asInt()); }
BLYNK_WRITE(V4){ setRelay(R5, param.asInt()); }
BLYNK_WRITE(V5){ setRelay(R6, param.asInt()); }
BLYNK_WRITE(V6){ setRelay(R7, param.asInt()); }
BLYNK_WRITE(V7){ setRelay(R8, param.asInt()); }
BLYNK_WRITE(V8){ setRelay(R9, param.asInt()); }
BLYNK_WRITE(V9){ setRelay(R10, param.asInt()); }

// =================================================

void setRelay(int pin, int state)
{
  digitalWrite(pin, state);
  if(pin == R1) lightState = state;
}

// =================================================
// ================= SENSOR DATA ===================
// =================================================

void sendSensorData()
{
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();
  float pressure = bmp.readPressure() / 100.0;

  if (isnan(humidity) || isnan(temperature))
  {
    Serial.println("DHT Error!");
    return;
  }

  // ---- Send to Blynk ----
  Blynk.virtualWrite(V10, temperature);
  Blynk.virtualWrite(V11, humidity);
  Blynk.virtualWrite(V12, pressure);

  // ---- Send to DWIN ----
  sendValue(0x6000, temperature * 10);
  sendValue(0x6100, humidity);
  sendValue(0x6200, pressure);

  Serial.println("Sensor Updated");
}

// =================================================
// ================= PIR LOGIC =====================
// =================================================

void checkPIR()
{
  int motion = digitalRead(PIR_PIN);

  if (motion == HIGH)
  {
    lastMotionTime = millis();

    if (!lightState)
    {
      setRelay(R1, HIGH);
      Blynk.virtualWrite(V0, 1);
    }
  }

  if (lightState && (millis() - lastMotionTime > motionDelay))
  {
    setRelay(R1, LOW);
    Blynk.virtualWrite(V0, 0);
  }
}

// =================================================
// ================= DWIN READ =====================
// =================================================

void readDWIN()
{
  while (dwin.available() >= 9)
  {
    if (dwin.read() == 0x5A)
    {
      if (dwin.peek() == 0xA5)
      {
        dwin.read();

        Buffer[0] = 0x5A;
        Buffer[1] = 0xA5;

        for (int i = 2; i < 9; i++)
          Buffer[i] = dwin.read();

        uint16_t vp = (Buffer[4] << 8) | Buffer[5];
        byte value = Buffer[8];

        handleDwinRelay(vp, value);
      }
    }
  }
}

// =================================================
// ================= DWIN RELAY ====================
// =================================================

void handleDwinRelay(uint16_t vp, byte value)
{
  switch (vp)
  {
    case 0x9000: setRelay(R1, value); Blynk.virtualWrite(V0, value); break;
    case 0x9100: setRelay(R2, value); Blynk.virtualWrite(V1, value); break;
    case 0x9200: setRelay(R3, value); Blynk.virtualWrite(V2, value); break;
    case 0x9300: setRelay(R4, value); Blynk.virtualWrite(V3, value); break;
    case 0x9400: setRelay(R5, value); Blynk.virtualWrite(V4, value); break;

    case 0x8000: setRelay(R6, value); Blynk.virtualWrite(V5, value); break;
    case 0x8100: setRelay(R7, value); Blynk.virtualWrite(V6, value); break;
    case 0x8200: setRelay(R8, value); Blynk.virtualWrite(V7, value); break;
    case 0x8300: setRelay(R9, value); Blynk.virtualWrite(V8, value); break;
    case 0x8400: setRelay(R10, value); Blynk.virtualWrite(V9, value); break;
  }
}

// =================================================
// ================= SEND VALUE ====================
// =================================================

void sendValue(uint16_t vp, int value)
{
  byte frame[8] = {
    0x5A, 0xA5, 0x05, 0x82,
    highByte(vp), lowByte(vp),
    highByte(value), lowByte(value)
  };

  dwin.write(frame, 8);
}

// =================================================

void setup()
{
  Serial.begin(115200);

  // DWIN UART2
  dwin.begin(115200, SERIAL_8N1, 16, 17);

  pinMode(PIR_PIN, INPUT);

  int relays[] = {R1,R2,R3,R4,R5,R6,R7,R8,R9,R10};
  for(int i=0;i<10;i++)
  {
    pinMode(relays[i], OUTPUT);
    digitalWrite(relays[i], LOW);
  }

  dht.begin();
  delay(2000);

  if (!bmp.begin())
  {
    Serial.println("BMP180 not found!");
    while (1);
  }

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  timer.setInterval(2000L, sendSensorData);
  timer.setInterval(500L, checkPIR);

  Serial.println("System Ready");
}

// =================================================

void loop()
{
  Blynk.run();
  timer.run();
  readDWIN();
}
