#include <WiFi.h>
#include <HTTPClient.h>
#include <SoftwareSerial.h>
#include <ArduinoJson.h>

const char *ssid = "Ismail";
const char *password = "147890147890";

// GSM module setup
#define RX_PIN 17       // Connect to TX of SIM900A
#define TX_PIN 16       // Connect to RX of SIM900A
HardwareSerial GSM(1);  // UART1 for GSM communication

String apiEndpoint = "http://188.166.251.135:8090/api/v1/order/sms";

String sendATCommand(const char *command, uint16_t delayMs = 1000) {
  GSM.println(command);  // Send AT command
  delay(delayMs);        // Wait for a response
  if (GSM.available()) {
    String sms = GSM.readString();  // Read GSM response
    Serial.print("\"");
    Serial.print(sms);  // Print GSM response to Serial Monitor
    Serial.println("\"");
    return sms;
  }
  return "";
}

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
  Serial.begin(115200);  // Serial Monitor
  // Initialize UART1 for GSM
  GSM.begin(9600, SERIAL_8N1, RX_PIN, TX_PIN);

  Serial.println("Initializing GSM module...");
  delay(3000);

  // Step 1: Set SMS mode to text
  sendATCommand("AT+CMGF=1");
  delay(100);

  // Step 2: Set SMS storage to internal memory
  sendATCommand("AT+CPMS=\"ME\"");
  delay(100);

  // Connect to WiFi
  connectToWiFi();
  delay(5000);
}


void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi disconnected, attempting to reconnect...");
    connectToWiFi();
  }
  String sms = sendATCommand("AT+CMGL=\"ALL\"", 1000);
  if (sms.length() > 0) {
    // chak is it a valid SMS response
    if (sms.indexOf("+CMGL:") != -1) {
      // then if ok, create json
      String jsonString = createJson(sms);
      // send to api
      String response = sendToAPI(jsonString);
      if (response.length() > 0) {
        // have a valid response
        // then delete all sms
        sendATCommand("AT+CMGD=1,4");
        // blink the Light for a sec
        digitalWrite(LED_BUILTIN, HIGH);
        delay(1000);
        digitalWrite(LED_BUILTIN, LOW);
      }
    } else {
      Serial.println("SMS may not valid. \"+CMGL:\" not found");
    }
  }
  delay(5000);
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

String createJson(const String &input) {
  // Create a dynamic JSON document
  DynamicJsonDocument doc(256);
  doc["sms"] = input;

  // Serialize the JSON object to a string
  String jsonString;
  serializeJson(doc, jsonString);
  return jsonString;
}
