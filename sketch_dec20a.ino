#include <WiFi.h>
#include <HTTPClient.h>
#include <SoftwareSerial.h>

const char* ssid = "Ismail";
const char* password = "147890147890";

// GSM module setup
#define RX_PIN 17  // Connect to TX of SIM900A
#define TX_PIN 16  // Connect to RX of SIM900A
HardwareSerial SIM900A(1);

void setup() {
    Serial.begin(115200); // Serial Monitor
    SIM900A.begin(9600, SERIAL_8N1, RX_PIN, TX_PIN); // Initialize GSM
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
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
    }
}


void loop() {
  // Check if GSM module sends data
  if (SIM900A.available()) {
    String sms = SIM900A.readString();
    Serial.println("Received SMS: ");
    Serial.println(sms);

    // Send SMS content to the API
    sendToAPI(sms);
  }
}

void sendToAPI(String smsText) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;

    // API URL with SMS text
    String apiUrl = "http://188.166.251.135:8090/api/v1/order/sms?text=" + urlencode(smsText);
    ;

    http.begin(apiUrl);

    // Make GET request
    int httpResponseCode = http.GET();
    if (httpResponseCode > 0) {
      Serial.println("API Response Code: " + String(httpResponseCode));
      String response = http.getString();
      Serial.println("API Response: " + response);
    } else {
      Serial.println("Error in API call: " + String(http.errorToString(httpResponseCode).c_str()));
      Serial.print("Error code : ");
      Serial.println(httpResponseCode);
    }
    http.end();
  } else {
    Serial.println("WiFi disconnected!");
  }
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
