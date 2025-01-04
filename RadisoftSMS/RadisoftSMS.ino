#include <WiFi.h>
#include <HTTPClient.h>
#include <SoftwareSerial.h>
#include <ArduinoJson.h>
#include <string>
#include <vector>

using namespace std;

vector<vector<string>> allSMSWithInfo;
int maxTimeDifference = 5;  // Maximum time difference in seconds

vector<string> extractHeaderInformation(const string &input) {
  vector<string> result;



  // Extract first number
  int firstNumberIndex = input.find("\"+88");
  if (firstNumberIndex != -1) {
    string firstNumber = input.substr(firstNumberIndex + 1, 14);
    result.push_back(firstNumber);
  } else {
    result.push_back("");
  }

  // Extract date
  int indexOfDate = input.find("/");
  if (indexOfDate != -1) {
    string date = input.substr(indexOfDate - 2, 8);
    result.push_back(date);
  } else {
    result.push_back("");
  }

  // Extract time
  int indexOfTime = input.find_last_of(":");
  if (indexOfTime != -1) {
    string time = input.substr(indexOfTime - 5, 8);
    result.push_back(time);
  } else {
    result.push_back("");
  }

  // Extract second number
  int indexOfSecondNumber = input.find_last_of("+88");
  if (indexOfSecondNumber != -1) {
    string secondNumber = input.substr(indexOfSecondNumber - 2, 14);
    result.push_back(secondNumber);
  } else {
    result.push_back("");
  }

  // Extract length of full text
  int indexOfLength = input.find_last_of(",");
  if (indexOfLength != -1) {
    string length = input.substr(indexOfLength + 1, 3);
    result.push_back(length);
  } else {
    result.push_back("");
  }
  return result;
}

bool isDifferenceLessThenXSeconds(const string &time1, const string &time2, int x) {
  // Extract hours, minutes, and seconds from time1
  int h1 = stoi(time1.substr(0, 2));
  int m1 = stoi(time1.substr(3, 5));
  int s1 = stoi(time1.substr(6, 8));

  // Extract hours, minutes, and seconds from time2
  int h2 = stoi(time2.substr(0, 2));
  int m2 = stoi(time2.substr(3, 5));
  int s2 = stoi(time2.substr(6, 8));

  // Convert the times into total seconds
  int totalSeconds1 = h1 * 3600 + m1 * 60 + s1;
  int totalSeconds2 = h2 * 3600 + m2 * 60 + s2;

  // Calculate the absolute difference in seconds
  int difference = abs(totalSeconds1 - totalSeconds2);

  // Return true if the difference is at least 5 seconds
  Serial.printf("Difference: %d\n", difference);
  return difference <= x;
}

int totalSMSsend = 0;
bool sendToAPI(vector<string> &headerWithSMSBody) {
  Serial.println("\nSending to API.....\n");
  for (int i = 0; i < headerWithSMSBody.size(); i++) {
    Serial.print(headerWithSMSBody[i].c_str());
    Serial.print(",");
  }
  Serial.println("\n\nSend is done\n");
  totalSMSsend++;
  Serial.printf("Total SMS send: %d", totalSMSsend);

  // +8801324204739,25/01/02,20:25:33,+8801700000600,102,255There we have a large SMS with 6 sentence.
  // There we have a large SMS with 6 sentence
  // There we have a large SMS with 6 sentence
  // There we have a large S
  // MS with 6 sentence
  // There we have a large SMS with 6 sentence
  // There we have a large SMS with 6 sentence

  DynamicJsonDocument doc(700);
  doc["number"] = headerWithSMSBody[0];
  doc["date"] = headerWithSMSBody[1];
  doc["time"] = headerWithSMSBody[2];
  doc["sms"] = headerWithSMSBody[5];


  // Serialize the JSON object to a string
  String jsonString;
  serializeJson(doc, jsonString);
  String feedback = sendToAPI(jsonString);
  if (feedback.length() > 0) {
    return true;
  } else {
    return true;
  }
}

bool isMatchedTwoHeader(vector<string> &vector1, vector<string> &vector2) {
  bool numberMatch = vector1[0].compare(vector2[0]) == 0;
  bool dateMatch = vector1[1].compare(vector2[1]) == 0;
  bool timeMatch = isDifferenceLessThenXSeconds(vector1[2], vector2[2], maxTimeDifference);
  bool secondNumberMatch = vector1[3].compare(vector2[3]) == 0;
  Serial.printf("Comparing: %d%d%d%d\n", numberMatch, dateMatch, timeMatch, secondNumberMatch);
  if (numberMatch && dateMatch && timeMatch && secondNumberMatch) {
    return true;
  } else {
    return false;
  }
}

bool isExitsInAllSMSVector(vector<string> &headerWithSMSBody) {
  for (int i = 0; i < allSMSWithInfo.size(); i++) {
    if (isMatchedTwoHeader(allSMSWithInfo[i], headerWithSMSBody)) {
      return true;
    }
  }
  return false;
}

bool searchForMainSMSAndInsert(vector<string> &headerWithSMSBody) {
  Serial.println("send to Searching for main SMS");
  Serial.println("Header with SMS body: ");
  for (int i = 0; i < headerWithSMSBody.size(); i++) {
    Serial.print(headerWithSMSBody[i].c_str());
  }
  Serial.println();
  Serial.println(allSMSWithInfo.size());
  for (int i = 0; i < allSMSWithInfo.size(); i++) {
    if (isMatchedTwoHeader(allSMSWithInfo[i], headerWithSMSBody)) {
      // found the main SMS
      Serial.println("Found the main SMS");
      string fullSMS = allSMSWithInfo[i][5] + headerWithSMSBody[5];
      int lenOfFullSMSOnSMSdata = stoi(allSMSWithInfo[i][6]);

      headerWithSMSBody.pop_back();
      headerWithSMSBody.push_back(fullSMS);

      Serial.print(fullSMS.c_str());
      Serial.print("---------");
      Serial.println(lenOfFullSMSOnSMSdata);

      if (fullSMS.length() >= lenOfFullSMSOnSMSdata) {
        // send to API
        sendToAPI(headerWithSMSBody);
        allSMSWithInfo.erase(allSMSWithInfo.begin() + i);
        return true;
      } else {
        allSMSWithInfo[i].erase(allSMSWithInfo[i].begin() + 5);
        allSMSWithInfo[i].insert(allSMSWithInfo[i].begin() + 5, fullSMS);
        Serial.println(allSMSWithInfo[i][5].c_str());
        Serial.println("Inserted again");
        return true;
      }
      return true;
    }
  }
  return false;
}

void checkAllSMSAndSend() {
  for (int i = 0; i < allSMSWithInfo.size(); i++) {
    int fullSMSlen = stoi(allSMSWithInfo[i][5]);
    string fullSMS = allSMSWithInfo[i][4];
    if (fullSMS.length() >= fullSMSlen) {
      sendToAPI(allSMSWithInfo[i]);
      allSMSWithInfo.erase(allSMSWithInfo.begin() + i);
    }
  }
}



void pushSmsAndCheck(string sms) {
  if ((sms.find("+CMT:") != -1) || (sms.find("+CMGL") != -1)) {
    sms = sms.substr(3, sms.length() - 1);
    int indexOfEndOfFirstLine = sms.find("\n");
    string header = sms.substr(0, indexOfEndOfFirstLine);
    string body = sms.substr(indexOfEndOfFirstLine + 1);
    vector<string> headerInfoWithBody = extractHeaderInformation(header);
    headerInfoWithBody.push_back(body);
    string first3Char = body.substr(0, 3);
    Serial.print("First 3 char are : ");
    Serial.println(first3Char.c_str());
    bool isExits = isExitsInAllSMSVector(headerInfoWithBody);
    Serial.printf("Is earlier exits in allSMSVector: %d\n", isExits);
    if ((!isExits) && first3Char[0] <= '9' && first3Char[0] >= '0' && first3Char[1] <= '9' && first3Char[1] >= '0' && first3Char[2] <= '9' && first3Char[2] >= '0') {
      int lenOfSMSUpcomingSMS = stoi(first3Char);
      Serial.printf("Length of SMS: %d\n", lenOfSMSUpcomingSMS);
      Serial.printf("Length of body: %d\n", body.length());
      if (lenOfSMSUpcomingSMS <= body.length()) {
        // sms is short. No need to another sms to complete
        // send To API and don't include to buffer
        sendToAPI(headerInfoWithBody);
        Serial.println("%d. SMS is short. Sending to API");
      } else {
        // sms have more parts to receive
        // wait for next part of SMS and send to buffer
        headerInfoWithBody.push_back(first3Char);
        allSMSWithInfo.push_back(headerInfoWithBody);
        Serial.println("SMS is long. Waiting for next part");
      }
    } else {

      // part of another SMS.
      // Search for that and insert
      // if not found, its not form the customers
      Serial.println("Part of another SMS");
      bool isFound = searchForMainSMSAndInsert(headerInfoWithBody);
      Serial.println("SMS is part of another SMS");
      if (!isFound) {
        // not found. Not from the customers
        sendToAPI(headerInfoWithBody);
        Serial.println("SMS is not from the customers. Sending to API");
      }
    }
  } else {
    Serial.println("Not a SMS response....");
  }
}



const char *ssid = "Ismail";
const char *password = "147890147890";

// GSM module setup
#define RX_PIN 17       // Connect to TX of SIM900A
#define TX_PIN 16       // Connect to RX of SIM900A
HardwareSerial GSM(1);  // UART1 for GSM communication

String apiEndpoint = "http://188.166.251.135:8090/api/v1/order/sms";

void sendATCommand(const char *command, uint16_t delayMs = 1000) {
  GSM.println(command);  // Send AT command
  delay(delayMs);        // Wait for a response
  while (GSM.available()) {
    String sms = GSM.readString();  // Read GSM response
    Serial.println(sms);            // Print GSM response to Serial Monitor
    pushSmsAndCheck(sms.c_str());
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

  // Step 3: List unread messages
  Serial.println("Reading unread messages...");
  sendATCommand("AT+CMGL=\"REC UNREAD\"");
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

unsigned long previousMillis = 0;     // Stores the last time the task was executed
const unsigned long interval = 1000;  // Interval in milliseconds (1 second)

unsigned long previousMillis2 = 0;     // Stores the last time the task was executed
const unsigned long interval2 = 5000;  // Interval in milliseconds (1 second)


void loop() {
  // Keep checking for responses or further actions
  while (GSM.available()) {
    Serial.println("Received SMS: ");
    String sms = GSM.readString();
    Serial.println(sms);  // Print GSM module responses
    // Send SMS content to the API
    // Serial.println("Sending to API");
    pushSmsAndCheck(sms.c_str());
  }

  unsigned long currentMillis = millis();  // Get the current time
  if (currentMillis - previousMillis2 >= interval2) {
    previousMillis2 = currentMillis;
    sendATCommand("AT+CMGL=\"REC UNREAD\"");
  }

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    Serial.print(".");
    // check allSMSWithInfo
    // if timeout for receiving SMS send uncompleted sms to server;
    // if found completion then send to api.
    if (allSMSWithInfo.size() > 0) {
      Serial.printf("Before checking all SMS with len %d\n", allSMSWithInfo.size());
      checkAllSMSAndSend();
      Serial.printf("After checking all SMS with len %d", allSMSWithInfo.size());

      // print allSMSWithInfo
      Serial.println("Printing allSMSWithInfo");
      for (int i = 0; i < allSMSWithInfo.size(); i++) {
        for (int j = 0; j < allSMSWithInfo[i].size(); j++) {
          Serial.print(allSMSWithInfo[i][j].c_str());
        }
      }
      Serial.println("\n\n--End of printing allSMSWithInfo\n\n");
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
