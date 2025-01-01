#include <WiFi.h>
#include <HTTPClient.h>
#include <SoftwareSerial.h>
#include <ArduinoJson.h>
#include <regex>
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>


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
    SIM900A.println("AT+CSMP=17,167,0,0"); // Enable concatenation
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

    String payload = createJson(smsText);
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

String createJson(const String &input) {
    // Create a dynamic JSON document
    DynamicJsonDocument doc(256);
    doc["sms"] = input;

    // Serialize the JSON object to a string
    String jsonString;
    serializeJson(doc, jsonString);
    return jsonString;
}


///////////////




using namespace std;

vector<string> extractHeaderInformation(const string &input)
{
    vector<string> result;

    // Regular expressions for each required field
    regex firstNumberRegex(R"(\+CMT: \"(\+\d+))");
    regex dateRegex(R"((\d{2}/\d{2}/\d{2}))");
    regex timeRegex(R"((\d{2}:\d{2}:\d{2}))");
    regex secondNumberRegex(R"(,\"(\+\d+)\".*?,\d+,\d+$)");
    regex lengthRegex(R"(,(\d+)\n)");

    smatch match;

    // Extract first number
    if (regex_search(input, match, firstNumberRegex))
    {
        result.push_back(match[1]); // First phone number
    }
    else
    {
        result.push_back("");
    }

    // Extract date
    if (regex_search(input, match, dateRegex))
    {
        result.push_back(match[1]); // Date
    }
    else
    {
        result.push_back("");
    }

    // Extract time
    if (regex_search(input, match, timeRegex))
    {
        result.push_back(match[1]); // Time
    }
    else
    {
        result.push_back("");
    }

    // Extract second number
    if (regex_search(input, match, secondNumberRegex))
    {
        result.push_back(match[1]); // Second phone number
    }
    else
    {
        result.push_back("");
    }

    // Extract length of full text
    if (regex_search(input, match, lengthRegex))
    {
        result.push_back(match[1]); // Length
    }
    else
    {
        result.push_back("");
    }

    return result;
}

bool isDifferenceAtLeastXSeconds(const std::string& time1, const std::string& time2, int difference) {
    int h1, m1, s1, h2, m2, s2;

    // Parse the time strings into hours, minutes, and seconds
    sscanf(time1.c_str(), "%d:%d:%d", &h1, &m1, &s1);
    sscanf(time2.c_str(), "%d:%d:%d", &h2, &m2, &s2);

    // Convert the times into total seconds
    int totalSeconds1 = h1 * 3600 + m1 * 60 + s1;
    int totalSeconds2 = h2 * 3600 + m2 * 60 + s2;

    // Calculate the absolute difference in seconds
    int difference = std::abs(totalSeconds1 - totalSeconds2);

    // Return true if the difference is at least 5 seconds, else false
    return difference >= difference;
}

bool searchForMainSMSAndInsert(vector<vector<string>> &allSMSVectorWithInfo, vector<string> &headerWithSMSBody)
{
    cout << "send to Searching for main SMS" << endl;
    cout << "Header with SMS body: " << endl;
    for (int i = 0; i < headerWithSMSBody.size(); i++)
    {
        cout << i << ". " << headerWithSMSBody[i] << " ";
    }
    cout << endl;
    for (int i = 0; i < allSMSVectorWithInfo.size(); i++)
    {
        vector<string> headerInfo = allSMSVectorWithInfo[i];
        bool numberMatch = headerInfo[0].compare(headerWithSMSBody[0]) == 0;
        bool dateMatch = headerInfo[1].compare(headerWithSMSBody[1]) == 0;
        bool timeMatch = headerInfo[2].compare(headerWithSMSBody[2]) == 0;
        bool secondNumberMatch = headerInfo[3].compare(headerWithSMSBody[3]) == 0;
        cout <<  headerInfo[2]  << endl<< headerWithSMSBody[2] << endl;
        cout << "Comparing: " << numberMatch << " " << dateMatch << " " << timeMatch << " " << secondNumberMatch << endl;
        if (numberMatch && dateMatch && timeMatch && secondNumberMatch)
        {
            // found the main SMS
            cout << "Found the main SMS" << endl;
            string fullSMS = allSMSVectorWithInfo[i][4] + headerWithSMSBody[4];
            allSMSVectorWithInfo[i].push_back(fullSMS);
            return true;
        }
    }
    return false;
}

bool sendToAPI(vector<string> &headerWithSMSBody)
{
    // TODO
    cout << "Sending to API....." << endl;
    for (int i = 0; i < headerWithSMSBody.size(); i++)
    {
        cout << headerWithSMSBody[i] << " ";
    }
    cout << endl;
    return true;
}

void checkAllSMSAndSend(vector<vector<string>> &allSMSWithInfo)
{
    for (int i = 0; i < allSMSWithInfo.size(); i++)
    {
        int fullSMSlen = stoi(allSMSWithInfo[i][5]);
        string fullSMS = allSMSWithInfo[i][4];
        if (fullSMS.length() >= fullSMSlen)
        {
            sendToAPI(allSMSWithInfo[i]);
            allSMSWithInfo.erase(allSMSWithInfo.begin() + i);
        }
    }
}

int xyz()
{
    vector<string> allSMS;
    string s1 = "+CMT: \"+8801324204739\",\"\",\"25/01/01,13:10:26+24\",145,32,0,0,\"+8801700000600\",145,153\n255There we have a large SMS with 6 sentence.\nThere we have a large SMS with 6 sentence\nThere we have a large SMS with 6 sentence\nThere we have a large S";
    string s2 = "+CMT: \"+8801324204739\",\"\",\"25/01/01,13:10:27+24\",145,32,0,0,\"+8801700000600\",145,99\nMS with 6 sentence\nThere we have a large SMS with 6 sentence\nThere we have a large SMS with 6 sentence";
    if (s1.find("+CMT:") != -1)
    {
        allSMS.push_back(s1);
    }
    if (s2.find("+CMT:") != -1)
    {
        allSMS.push_back(s2);
    }

    vector<vector<string>> allSMSWithInfo;

    for (int i = 0; i < allSMS.size(); i++)
    {
        string sms = allSMS[i];
        int indexOfEndOfFirstLine = sms.find("\n");
        string header = sms.substr(0, indexOfEndOfFirstLine);
        string body = sms.substr(indexOfEndOfFirstLine + 1);
        vector<string> headerInfoWithBody = extractHeaderInformation(header);
        headerInfoWithBody.push_back(body);
        string first3Char = body.substr(0, 3);
        if (first3Char[0] <= '9' && first3Char[0] >= '0' && first3Char[1] <= '9' && first3Char[1] >= '0' && first3Char[2] <= '9' && first3Char[2] >= '0')
        {
            int lenOfSMSUpcomingSMS = stoi(first3Char);
            cout << "Length of SMS: " << lenOfSMSUpcomingSMS << endl;
            cout << "Length of body: " << body.length() << endl;
            if (lenOfSMSUpcomingSMS <= body.length())
            {
                // sms is short. No need to another sms to complete
                // send To API and don't include to buffer
                sendToAPI(headerInfoWithBody);
                cout << i << ". SMS is short. Sending to API" << endl;
            }
            else
            {
                // sms have more parts to receive
                // wait for next part of SMS and send to buffer
                headerInfoWithBody.push_back(first3Char);
                allSMSWithInfo.push_back(headerInfoWithBody);
                cout << i << ". SMS is long. Waiting for next part" << endl;
            }
        }
        else
        {
            // part of another SMS.
            // Search for that and insert
            // if not found, its not form the customers
            bool isFound = searchForMainSMSAndInsert(allSMSWithInfo, headerInfoWithBody);
            cout << i << ". SMS is part of another SMS" << endl;
            if (!isFound)
            {
                // not found. Not from the customers
                sendToAPI(headerInfoWithBody);
                cout << i << ". SMS is not from the customers. Sending to API" << endl;
            }
        }

        // check allSMSWithInfo
        // if timeout for receiving SMS send uncompleted sms to server;
        // if found completion then send to api.
        cout << i << ". Before checking all SMS with len " << allSMSWithInfo.size() << endl;
        checkAllSMSAndSend(allSMSWithInfo);
        cout << i << ". After checking all SMS with len " << allSMSWithInfo.size() << endl;

        // print allSMSWithInfo
        cout << "Printing allSMSWithInfo" << endl;
        for (int i = 0; i < allSMSWithInfo.size(); i++)
        {
            for (int j = 0; j < allSMSWithInfo[i].size(); j++)
            {
                cout << allSMSWithInfo[i][j] << " ";
            }
        }
        cout << "\n\n--End of printing allSMSWithInfo" << endl
             << endl
             << endl;
    }

    return 0;
}

