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

const char* ssid = "home305";
const char* password = "idontknow";

HardwareSerial mySerial(1);  // UART instance (UART1, TX=17, RX=16)

// Function to handle file upload and transfer it over UART
void handleFileUpload(AsyncWebServerRequest *request, const String& filename, size_t index, uint8_t *data, size_t len, bool final) {
  mySerial.write(data, len);  // Send the received chunk over UART
  
  // Used to verify if .bin file was received or not.
  //Serial.write(data, len);

  if (final) {
    Serial.println("\n");
    Serial.println("File transfer completed.");
    
    // Response back to the client to inform about completion
    String html = "<html><body>"
                  "<h2>File Upload Complete</h2>"
                  "<p>File transfer completed successfully!</p>"
                  "<a href='/'>Go Back</a>"
                  "</body></html>";
    request->send(200, "text/html", html);
  }
}

void setup() {
    Serial.begin(115200);

    // Initialize UART for communication with STM32
    mySerial.begin(115200, SERIAL_8N1, 16, 17);

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);

    if (WiFi.waitForConnectResult() != WL_CONNECTED) {
        Serial.printf("WiFi Failed!\n");
        return;
    }

    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());

    // Server form
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        String html = "<html><body>"
                      "<h2>Upload .bin File</h2>"
                      "<form method='POST' enctype='multipart/form-data' action='/upload'>"
                      "<input type='file' name='file' accept='.bin'><br><br>"
                      "<input type='submit' value='Upload'>"
                      "</form>"
                      "</body></html>";
        request->send(200, "text/html", html);
    });

    server.begin();
}

void loop() {}
