#include <iostream>
#include <string>
#include <vector>
// #include <algorithm>

using namespace std;

vector<vector<string>> allSMSWithInfo;
int maxTimeDifference = 5; // Maximum time difference in seconds

vector<string> extractHeaderInformation(const string &input)
{
    vector<string> result;



    // Extract first number
    int firstNumberIndex = input.find("\"+88");
    if(firstNumberIndex != -1){
        string firstNumber = input.substr(firstNumberIndex + 1, 14);
        result.push_back(firstNumber);
    }
    else{
        result.push_back("");
    }

    // Extract date
    int indexOfDate = input.find("/");
    if(indexOfDate != -1){
        string date = input.substr(indexOfDate - 2, 8);
        result.push_back(date);
    }
    else{
        result.push_back("");
    }

    // Extract time
    int indexOfTime = input.find_last_of(":");
    if(indexOfTime != -1){
        string time = input.substr(indexOfTime - 5, 8);
        result.push_back(time);
    }
    else{
        result.push_back("");
    }

    // Extract second number
    int indexOfSecondNumber = input.find_last_of("+88");
    if(indexOfSecondNumber != -1){
        string secondNumber = input.substr(indexOfSecondNumber-2, 14);
        result.push_back(secondNumber);
    }
    else{
        result.push_back("");
    }

    // Extract length of full text
    int indexOfLength = input.find_last_of(",");
    if(indexOfLength != -1){
        string length = input.substr(indexOfLength + 1, 3);
        result.push_back(length);
    }
    else{
        result.push_back("");
    }
    return result;
}

bool isDifferenceLessThenXSeconds(const string &time1, const string &time2, int x)
{
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
    cout << "Difference: " << difference << endl;
    return difference <= x;
}

int totalSMSsend = 0;
bool sendToAPI(vector<string> &headerWithSMSBody)
{
    // TODO
    cout << "\nSending to API.....\n"
         << endl;
    for (int i = 0; i < headerWithSMSBody.size(); i++)
    {
        cout << headerWithSMSBody[i] << " ";
    }
    cout << "\n\nSend is done\n"
         << endl;
    totalSMSsend++;
    cout << "Total SMS send: " << totalSMSsend << endl;
    return true;
}

bool isMatchedTwoHeader(vector<string> &vector1, vector<string> &vector2)
{
    cout << "-----------------------isMatchedTwoHeader" << endl;
    bool numberMatch = vector1[0].compare(vector2[0]) == 0;
    bool dateMatch = vector1[1].compare(vector2[1]) == 0;
    bool timeMatch = isDifferenceLessThenXSeconds(vector1[2], vector2[2], maxTimeDifference);
    bool secondNumberMatch = vector1[3].compare(vector2[3]) == 0;
    cout << "Comparing: " << numberMatch << dateMatch << timeMatch << secondNumberMatch << endl;
    if (numberMatch && dateMatch && timeMatch && secondNumberMatch)
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool isExitsInAllSMSVector( vector<string> &headerWithSMSBody)
{
    for (int i = 0; i < allSMSWithInfo.size(); i++)
    {
        if (isMatchedTwoHeader(allSMSWithInfo[i], headerWithSMSBody))
        {
            return true;
        }
    }
    return false;
}

bool searchForMainSMSAndInsert( vector<string> &headerWithSMSBody)
{
    cout << "send to Searching for main SMS" << endl;
    cout << "Header with SMS body: " << endl;
    for (int i = 0; i < headerWithSMSBody.size(); i++)
    {
        cout << i << ". " << headerWithSMSBody[i] << " ";
    }
    cout << endl;
    cout << allSMSWithInfo.size() << endl;
    for (int i = 0; i < allSMSWithInfo.size(); i++)
    {
        if (isMatchedTwoHeader(allSMSWithInfo[i], headerWithSMSBody))
        {
            // found the main SMS
            cout << "Found the main SMS" << endl;
            string fullSMS = allSMSWithInfo[i][5] + headerWithSMSBody[5];
            int lenOfFullSMSOnSMSdata = stoi(allSMSWithInfo[i][6]);

            headerWithSMSBody.pop_back();
            headerWithSMSBody.push_back(fullSMS);

            cout << fullSMS << "---------" << lenOfFullSMSOnSMSdata << endl;

            if (fullSMS.length() >= lenOfFullSMSOnSMSdata)
            {
                // send to API
                sendToAPI(headerWithSMSBody);
                allSMSWithInfo.erase(allSMSWithInfo.begin() + i);
                return true;
            }
            else
            {
                allSMSWithInfo[i].erase(allSMSWithInfo[i].begin() + 5);
                allSMSWithInfo[i].insert(allSMSWithInfo[i].begin() + 5, fullSMS);
                cout << allSMSWithInfo[i][5] << endl;
                cout << "Inserted again" << endl;
                return true;
            }
            return true;
        }
    }
    return false;
}

void checkAllSMSAndSend()
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

int main()
{
    vector<string> allSMS;
    string s1 = "+CMT: \"+8801324204739\",\"\",\"25/01/01,13:10:26+24\",145,32,0,0,\"+8801700000600\",145,153\n255There we have a large SMS with 6 sentence.\nThere we have a large SMS with 6 sentence\nThere we have a large SMS with 6 sentence\nThere we have a large S";
    string s2 = "+CMT: \"+8801324204739\",\"\",\"25/01/01,13:10:27+24\",145,32,0,0,\"+8801700000600\",145,99\nMS with 6 sentence\nThere we have a large SMS with 6 sentence";
    string s3 = "+CMT: \"+8801324204739\",\"\",\"25/01/01,13:10:27+24\",145,32,0,0,\"+8801700000600\",145,99\n\nThere we have a large SMS with 6 sentence";

        allSMS.push_back(s1);
        allSMS.push_back(s2);
        allSMS.push_back(s3);
    
    s1 = "+CMT: \"+8801324204739\",\"\",\"25/01/01,13:10:56+24\",145,32,0,0,\"+8801700000600\",145,153\n255There we have a large SMS with 6 sentence.\nThere we have a large SMS with 6 sentence\nThere we have a large SMS with 6 sentence\nThere we have a large S";
    s2 = "+CMT: \"+8801324204739\",\"\",\"25/01/01,13:10:56+24\",145,32,0,0,\"+8801700000600\",145,99\nMS with 6 sentence\nThere we have a large SMS with 6 sentence";
    s3 = "+CMT: \"+8801324204739\",\"\",\"25/01/01,13:10:57+24\",145,32,0,0,\"+8801700000600\",145,99\n\nThere we have a large SMS with 6 sentence";

    

        allSMS.push_back(s1);
        allSMS.push_back(s2);
        allSMS.push_back(s3.c_str());

        // allSMS.push_back(s1);
        // allSMS.push_back(s2);
        // allSMS.push_back(s3);


    for (int i = 0; i < allSMS.size(); i++)
    {
        string sms = allSMS[i];
        int indexOfEndOfFirstLine = sms.find("\n");
        string header = sms.substr(0, indexOfEndOfFirstLine);
        string body = sms.substr(indexOfEndOfFirstLine + 1);
        vector<string> headerInfoWithBody = extractHeaderInformation(header);
        headerInfoWithBody.push_back(body);
        string first3Char = body.substr(0, 3);
        cout << first3Char << endl;
        if ((!isExitsInAllSMSVector(headerInfoWithBody)) && first3Char[0] <= '9' && first3Char[0] >= '0' && first3Char[1] <= '9' && first3Char[1] >= '0' && first3Char[2] <= '9' && first3Char[2] >= '0')
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
            cout << "Part of another SMS" << endl;
            bool isFound = searchForMainSMSAndInsert(headerInfoWithBody);
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
        checkAllSMSAndSend();
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

    cout << "\n\nEnd of the program" << endl;
    cout << "Length is : " << allSMSWithInfo.size() << endl;
    cout << "Total SMS send: " << totalSMSsend << endl;

    return 0;
}
