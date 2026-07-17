#include <Adafruit_NeoPixel.h>

#define V1_PIN            0 
#define H1_PIN            1  
#define V2_PIN            2  
#define H2_PIN            3   
#define SEL2_PIN          4  
#define SW2_PIN           6  
#define LED2_PIN          7 
#define LED1_PIN          8   
#define SW1_PIN           9   
#define RXD_PIN           10  
#define TXD_PIN           20  
#define SEL1_PIN          21  

#define CRSF_BAUD         420000

#define CRSF_SYNC_BYTE                     0xC8
#define CRSF_FRAMETYPE_RC_CHANNELS_PACKED  0x16
#define CRSF_FRAMETYPE_LINK_STATISTICS     0x14
#define CRSF_CHANNEL_VALUE_MIN   172   
#define CRSF_CHANNEL_VALUE_MID   992   
#define CRSF_CHANNEL_VALUE_MAX   1811  

#define ELRS_ADDRESS          0xEE
#define ADDR_RADIO             0xEA
#define ELRS_BIND_COMMAND      0xFF
#define TYPE_SETTINGS_WRITE    0x2D

Adafruit_NeoPixel led1(1, LED1_PIN, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel led2(1, LED2_PIN, NEO_GRB + NEO_KHZ800);

uint16_t channels[16];

uint8_t crc8_dvb_s2(uint8_t crc, uint8_t a) {
  crc ^= a;
  for (int i = 0; i < 8; i++) {
    crc = (crc & 0x80) ? (crc << 1) ^ 0xD5 : (crc << 1);
  }
  return crc;
}

uint8_t crc8_buf(uint8_t *data, uint8_t len) {
  uint8_t crc = 0;
  for (uint8_t i = 0; i < len; i++) crc = crc8_dvb_s2(crc, data[i]);
  return crc;
}

uint16_t adcToCrsf(int adcValue, int adcMin, int adcMid, int adcMax) {
  adcValue = constrain(adcValue, adcMin, adcMax);
  if (adcValue < adcMid) {
    return map(adcValue, adcMin, adcMid, CRSF_CHANNEL_VALUE_MIN, CRSF_CHANNEL_VALUE_MID);
  }
  return map(adcValue, adcMid, adcMax, CRSF_CHANNEL_VALUE_MID, CRSF_CHANNEL_VALUE_MAX);
}

void sendCrsfChannels() {
  uint8_t frame[26];
  uint8_t payload[22];
  memset(payload, 0, sizeof(payload));

  uint32_t bitBuffer = 0;
  int bitsInBuffer = 0;
  int byteIndex = 0;

  for (int ch = 0; ch < 16; ch++) {
    bitBuffer |= ((uint32_t)(channels[ch] & 0x7FF)) << bitsInBuffer;
    bitsInBuffer += 11;
    while (bitsInBuffer >= 8) {
      payload[byteIndex++] = bitBuffer & 0xFF;
      bitBuffer >>= 8;
      bitsInBuffer -= 8;
    }
  }
  if (bitsInBuffer > 0) payload[byteIndex++] = bitBuffer & 0xFF;

  frame[0] = CRSF_SYNC_BYTE;
  frame[1] = 22 + 2;
  frame[2] = CRSF_FRAMETYPE_RC_CHANNELS_PACKED;
  memcpy(&frame[3], payload, 22);
  frame[25] = crc8_buf(&frame[2], 23);

  Serial1.write(frame, 26);
}

void sendBindCommand() {
  uint8_t packet[8];
  packet[0] = ELRS_ADDRESS;
  packet[1] = 6;
  packet[2] = TYPE_SETTINGS_WRITE;
  packet[3] = ELRS_ADDRESS;
  packet[4] = ADDR_RADIO;
  packet[5] = ELRS_BIND_COMMAND;
  packet[6] = 0x01;
  packet[7] = crc8_buf(&packet[2], 5);
  Serial1.write(packet, 8);
}

void updatePowerLED() {
  led1.setPixelColor(0, led1.Color(40, 40, 40));
  led1.show();
}

volatile bool droneConnected = false;
volatile unsigned long lastTelemetryMillis = 0;
#define TELEMETRY_TIMEOUT_MS 1000

void updateConnectionLED() {
  bool connected = droneConnected && (millis() - lastTelemetryMillis < TELEMETRY_TIMEOUT_MS);
  led2.setPixelColor(0, connected ? led2.Color(0, 255, 0) : led2.Color(255, 0, 0));
  led2.show();
}

uint8_t rxBuf[64];
uint8_t rxIndex = 0;

void pollCrsfTelemetry() {
  while (Serial1.available()) {
    uint8_t b = Serial1.read();
    if (rxIndex == 0 && b != CRSF_SYNC_BYTE) continue;
    rxBuf[rxIndex++] = b;

    if (rxIndex >= 2) {
      uint8_t frameLen = rxBuf[1];
      if (rxIndex >= frameLen + 2) {
        uint8_t expectedCrc = rxBuf[frameLen + 1];
        uint8_t actualCrc = crc8_buf(&rxBuf[2], frameLen - 1);
        if (expectedCrc == actualCrc && rxBuf[2] == CRSF_FRAMETYPE_LINK_STATISTICS) {
          droneConnected = true;
          lastTelemetryMillis = millis();
        }
        rxIndex = 0;
      }
    }
    if (rxIndex >= sizeof(rxBuf)) rxIndex = 0;
  }
}

bool lastSW2State = HIGH;
bool lastSW1State = HIGH;
unsigned long sw1PressStart = 0;
#define SW1_LONGPRESS_MS 1500

void handleButtons() {
  bool sw2State = digitalRead(SW2_PIN);
  if (sw2State == LOW && lastSW2State == HIGH) {
    sendBindCommand();
  }
  lastSW2State = sw2State;

  bool sw1State = digitalRead(SW1_PIN);
  if (sw1State == LOW && lastSW1State == HIGH) {
    sw1PressStart = millis();
  }
  if (sw1State == LOW && (millis() - sw1PressStart > SW1_LONGPRESS_MS)) {
    led1.setPixelColor(0, 0); led1.show();
    led2.setPixelColor(0, 0); led2.show();
    gpio_wakeup_enable((gpio_num_t)SW1_PIN, GPIO_INTR_LOW_LEVEL);
    esp_sleep_enable_gpio_wakeup();
    esp_deep_sleep_start();
  }
  lastSW1State = sw1State;

  channels[6] = (digitalRead(SEL1_PIN) == LOW) ? CRSF_CHANNEL_VALUE_MAX : CRSF_CHANNEL_VALUE_MIN;
  channels[7] = (digitalRead(SEL2_PIN) == LOW) ? CRSF_CHANNEL_VALUE_MAX : CRSF_CHANNEL_VALUE_MIN;
}

void readJoysticks() {
  int adcMin = 100, adcMid = 2048, adcMax = 3995;

  int v1Raw = analogRead(V1_PIN);
  int h1Raw = analogRead(H1_PIN);
  int v2Raw = analogRead(V2_PIN);
  int h2Raw = analogRead(H2_PIN);

  channels[0] = adcToCrsf(h2Raw, adcMin, adcMid, adcMax); 
  channels[1] = adcToCrsf(v2Raw, adcMin, adcMid, adcMax); 
  channels[2] = adcToCrsf(v1Raw, adcMin, adcMid, adcMax); 
  channels[3] = adcToCrsf(h1Raw, adcMin, adcMid, adcMax); 
}

void setup() {
  Serial1.begin(CRSF_BAUD, SERIAL_8N1, RXD_PIN, TXD_PIN);

  pinMode(SW1_PIN, INPUT_PULLUP);
  pinMode(SW2_PIN, INPUT_PULLUP);
  pinMode(SEL1_PIN, INPUT_PULLUP);
  pinMode(SEL2_PIN, INPUT_PULLUP);

  led1.begin();
  led2.begin();
  led1.setBrightness(80);
  led2.setBrightness(80);
  led1.show();
  led2.show();

  for (int i = 0; i < 16; i++) channels[i] = CRSF_CHANNEL_VALUE_MID;
}

unsigned long lastChannelSend = 0;
#define CHANNEL_SEND_INTERVAL_MS 4 

void loop() {
  pollCrsfTelemetry();
  handleButtons();
  readJoysticks();

  if (millis() - lastChannelSend >= CHANNEL_SEND_INTERVAL_MS) {
    sendCrsfChannels();
    lastChannelSend = millis();
  }

  updatePowerLED();
  updateConnectionLED();
}
