#include <Arduino.h>
#ifdef ESP32
  #include <WiFi.h>
  #include <AsyncTCP.h>
#elif defined(ESP8266)
  #include <ESP8266WiFi.h>
  #include <ESPAsyncTCP.h>
#endif
#include <ESPAsyncWebServer.h>

AsyncWebServer server(80);
HardwareSerial mySerial(1);  // UART1: TX=17, RX=16

const char* ssid     = "Varun";
const char* password = "123456789";

static const size_t WINDOW_SIZE = 128;
uint8_t  windowBuf[WINDOW_SIZE];
size_t   windowIdx = 0;
size_t   totalBytes = 0;

// same 32-shift CRC you used in Python
uint32_t computeCRC(const uint8_t* buff, size_t len) {
  uint32_t crc = 0xFFFFFFFFUL;
  for (size_t i = 0; i < len; i++) {
    crc ^= buff[i];
    for (uint8_t b = 0; b < 32; b++) {
      if (crc & 0x80000000UL) crc = (crc << 1) ^ 0x04C11DB7UL;
      else                   crc <<= 1;
    }
  }
  return crc;
}

// called repeatedly as chunks arrive
void handleFileUpload(AsyncWebServerRequest *request,
                      const String& filename,
                      size_t index, uint8_t *data,
                      size_t len, bool final) {

  // on first chunk reset counters
  if (index == 0) {
    windowIdx   = 0;
    totalBytes  = 0;
    Serial.printf("Uploading %s…\n", filename.c_str());
  }

  // process each byte
  for (size_t i = 0; i < len; i++) {
    windowBuf[windowIdx++] = data[i];
    totalBytes++;

    // once we have WINDOW_SIZE bytes, send + CRC
    if (windowIdx == WINDOW_SIZE) {
      uint32_t crc = computeCRC(windowBuf, WINDOW_SIZE);
      mySerial.write(windowBuf, WINDOW_SIZE);
      mySerial.write((uint8_t*)&crc, 4);
      delay(200); 
      windowIdx = 0;
      Serial.printf("Sent %u bytes so far\n", (unsigned)totalBytes);
    }
  }

  // if this is the last chunk, flush any leftover bytes
  if (final) {
    if (windowIdx > 0) {
      uint32_t crc = computeCRC(windowBuf, windowIdx);
      mySerial.write(windowBuf, windowIdx);
      mySerial.write((uint8_t*)&crc, 4);
      Serial.printf("Flushed final %u bytes\n", (unsigned)windowIdx);
    }
    // send the 0x42 “update” command + its CRC
    uint8_t cmd = 0x42;
    uint32_t cmdCrc = computeCRC(&cmd, 1);
    mySerial.write(&cmd, 1);
    mySerial.write((uint8_t*)&cmdCrc, 4);
    Serial.println("Sent update command.");

    // reply to browser
    String html = "<html><body>"
                  "<h2>Upload Complete</h2>"
                  "<p>Transferred " + String(totalBytes) + " bytes.</p>"
                  "<a href='/'>Back</a>"
                  "</body></html>";
    request->send(200, "text/html", html);
  }
}

void setup() {
  Serial.begin(115200);
  mySerial.begin(115200, SERIAL_8N1, /*RX*/16, /*TX*/17);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Wi-Fi connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print('.');
  }
  Serial.printf("\nIP address: %s\n", WiFi.localIP().toString().c_str());

  // serve the upload form
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *req){
    req->send(200, "text/html",
      "<html><body>"
      "<h2>Upload a .bin File</h2>"
      "<form method='POST' enctype='multipart/form-data' action='/upload'>"
      "<input type='file' name='file' accept='.bin'><br><br>"
      "<input type='submit' value='Upload'>"
      "</form>"
      "</body></html>"
    );
  });

  // hook up the file‐upload handler
  server.on("/upload", HTTP_POST, 
    /* onDone */    [](AsyncWebServerRequest *r){}, 
    /* onUpload */  handleFileUpload
  );

  server.begin();
}

void loop() {
  // nothing else to do here
}
