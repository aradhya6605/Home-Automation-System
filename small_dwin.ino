HardwareSerial dwin(2);

#define RXD2 16
#define TXD2 17

// Lights
#define L1 13
#define L2 14
#define L3 27
#define L4 26
#define L5 25
#define L6 33
#define L7 32
#define L8 23

uint8_t buffer[20];

void setup() {
  Serial.begin(115200);
  dwin.begin(115200, SERIAL_8N1, RXD2, TXD2);

  pinMode(L1, OUTPUT);
  pinMode(L2, OUTPUT);
  pinMode(L3, OUTPUT);
  pinMode(L4, OUTPUT);
  pinMode(L5, OUTPUT);
  pinMode(L6, OUTPUT);
  pinMode(L7, OUTPUT);
  pinMode(L8, OUTPUT);
}

void loop() {

  if (dwin.available()) {
    delay(10);

    int i = 0;
    while (dwin.available() && i < 20) {
      buffer[i++] = dwin.read();
    }

    if (buffer[0] == 0x5A && buffer[1] == 0xA5) {

      uint16_t vp = buffer[4] << 8 | buffer[5];
      uint16_t value = buffer[7] << 8 | buffer[8];

      Serial.print("VP: ");
      Serial.print(vp, HEX);
      Serial.print(" Value: ");
      Serial.println(value);

      // 🔥 Bedroom / First Section
      if (vp == 0x6000) digitalWrite(L1, value);
      if (vp == 0x6100) digitalWrite(L2, value);
      if (vp == 0x6200) digitalWrite(L3, value);
      if (vp == 0x6300) digitalWrite(L4, value);

      // 🔥 Living Room / Second Section
      if (vp == 0x7100) digitalWrite(L5, value);
      if (vp == 0x7200) digitalWrite(L6, value);
      if (vp == 0x7300) digitalWrite(L7, value);
      if (vp == 0x7400) digitalWrite(L8, value);
    }
  }
}
