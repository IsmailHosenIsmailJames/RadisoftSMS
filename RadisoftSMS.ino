#include <WiFi.h>
#include <HTTPClient.h>
#include <SoftwareSerial.h>

const char* ssid = "Ismail";
const char* password = "147890147890";

// GSM module setup
#define RX_PIN 17  // Connect to TX of SIM900A
#define TX_PIN 16  // Connect to RX of SIM900A
SoftwareSerial SIM900A(RX_PIN, TX_PIN);

String apiEndpoint = "http://188.166.251.135:8090/api/v1/order/sms";

void setup() {
    Serial.begin(9600); // Serial Monitor
    SIM900A.begin(9600); // Initialize GSM
    delay(3000);

    // Set GSM text mode and encoding
    SIM900A.println("AT+CMGF=1"); // Text mode
    delay(100);
    SIM900A.println("AT+CSCS=\"GSM\""); // ASCII encoding
    delay(100);
    SIM900A.println("AT+CNMI=1,2,0,0,0"); // Forward SMS to ESP32
    delay(100);

    // Connect to WiFi
    WiFi.begin(ssid, password);
    Serial.print("Connecting to WiFi.");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWiFi connected!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
    delay(5000);
}


void loop() {
  // Check if GSM module sends data
  if (SIM900A.available()) {
    String sms = SIM900A.readString();
    Serial.println("Received SMS: ");
    Serial.println(sms);

    // Send SMS content to the API
    Serial.println("Sending to API");
    sendToAPI(sms);
  }
}

String sendToAPI(String smsText) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(apiEndpoint);
    http.addHeader("Content-Type", "application/json");

    String payload = "{\n\"sms\" : \""+smsText+"\"\n}";
    Serial.println("Sending to " + apiEndpoint);
    Serial.println(payload);
    int httpResponseCode = http.POST(payload);

    if (httpResponseCode == 200) {
      String response = http.getString();
      Serial.println("Server Response:");
      Serial.println(response);
      http.end();
      digitalWrite(LED_BUILTIN, HIGH);
      delay(200);
      digitalWrite(LED_BUILTIN, LOW);
      return response;
    } else {
      Serial.print("Error in HTTP request : ");
      Serial.println(httpResponseCode);
      Serial.println(String(http.errorToString(httpResponseCode).c_str()));
    }
    http.end();
  }
  return "";
}

String urlencode(String str) {
  String encodedString = "";
  char c;
  char code0;
  char code1;
  for (int i = 0; i < str.length(); i++) {
    c = str.charAt(i);
    if (isalnum(c)) {
      encodedString += c;
    } else {
      code0 = (c >> 4) & 0xF;
      code1 = c & 0xF;
      encodedString += '%';
      encodedString += (code0 < 10) ? char(code0 + '0') : char(code0 - 10 + 'A');
      encodedString += (code1 < 10) ? char(code1 + '0') : char(code1 - 10 + 'A');
    }
  }
  return encodedString;
}
