#include <WiFi.h>
#include <HTTPClient.h>
#include <SoftwareSerial.h>
#include <ArduinoJson.h>


// Define software serial pins
#define SIM900A_RX 17  // Example pin
#define SIM900A_TX 16  // Example pin

SoftwareSerial sim900a(SIM900A_RX, SIM900A_TX);

const char *ssid = "Ismail";
const char *password = "147890147890";

// GSM module setup
#define RX_PIN 17       // Connect to TX of SIM900A
#define TX_PIN 16       // Connect to RX of SIM900A
HardwareSerial GSM(1);  // UART1 for GSM communication

String apiEndpoint = "http://188.166.251.135:8090/api/v1/order/sms";

void connectToWiFi() {
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");

  // Try to connect for a maximum of 10 seconds
  unsigned long startTime = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startTime < 10000) {
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi connected!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nFailed to connect to WiFi.");
  }
}


void setup() {
  Serial.begin(9600);  // For debugging
  sim900a.begin(9600);   // SIM900A baud rate

  delay(1000);
  sim900a.println("AT");  // Test connection
  delay(1000);
  sim900a.println("AT+CMGF=1");  // Set text mode
  delay(1000);
  sim900a.println("AT+CNMI=2,2,0,0,0");  // New message indication
  delay(1000);
  connectToWiFi();
}

bool foundHeader = false;

String fullSMS = "";
void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi disconnected, attempting to reconnect...");
    connectToWiFi();
  }
  if (sim900a.available() > 0) {
    String smsData = sim900a.readStringUntil('\n');
    if (foundHeader) {
      fullSMS += smsData;
      if (smsData.lastIndexOf("END") != -1) {
        digitalWrite(LED_BUILTIN, HIGH);
        Serial.println("Send full sms to api : " + fullSMS);
        String jsonString = createJson(fullSMS);
        while (fullSMS.length() > 0) {
          Serial.print(".");
          String response = sendToAPI(jsonString);
          if (response.isEmpty()) {
            Serial.print("Someting went wrong");
          } else {
            fullSMS = "";
          }
        }
        digitalWrite(LED_BUILTIN, LOW);
        Serial.println("Successfully send to api");
      }
      
      Serial.println("body : " + smsData);
      foundHeader = false;
    }
    if (smsData.startsWith("+CMT:")) {
      Serial.println("header : " + smsData);
      fullSMS += smsData;
      foundHeader = true;
    }
  }
}


String sendToAPI(String payload) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(apiEndpoint);
    http.addHeader("Content-Type", "application/json");

    Serial.println("Sending to " + apiEndpoint);
    Serial.println(payload);
    int httpResponseCode = http.POST(payload);

    if (httpResponseCode == 200) {
      String response = http.getString();
      Serial.println("Server Response:");
      Serial.println(response);
      http.end();
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

String createJson(const String &sms) {
  // Create a dynamic JSON document
  DynamicJsonDocument doc(256);
  doc["sms"] = sms;

  // Serialize the JSON object to a string
  String jsonString;
  serializeJson(doc, jsonString);
  return jsonString;
}
