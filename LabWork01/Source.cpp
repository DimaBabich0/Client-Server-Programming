#include <iostream>
#include <windows.h>
#include <string>
#include <fstream>
#pragma comment(lib, "urlmon.lib")
using namespace std;
const int STR_SIZE = 256;

string FindKey(string message, string key)
{
    string value;
    size_t pos = message.find(key);
    if (pos != wstring::npos)
    {
        pos += key.size();
        pos += 1;
        if (message[pos] == '"' ||
            message[pos] == ' ')
            pos++;
        while (TRUE)
        {
            if (message[pos] == '"' ||
                message[pos] == ',' ||
                message[pos] == '}' ||
                message[pos] == '\n')
                break;
            value += message[pos];
            pos++;
        }
    }
    return value;
}

void DownloadFile(string uValcode, string uDate, string destFile)
{
	// the URL to download from 
	string valcode = "valcode=";
	valcode += uValcode;

	string date = "&date=";
	date += uDate;

	string srcURL = "https://bank.gov.ua/NBUStatService/v1/statdirectory/exchange?" + valcode + date + "&json";

	// URLDownloadToFile returns S_OK on success 
	if (URLDownloadToFileA(NULL, srcURL.c_str(), destFile.c_str(), 0, NULL) != S_OK)
		cout << "Error. Can't save the file\n";
}

int main()
{
    setlocale(0, "ru");
    string date;
    
    while (true)
    {
        cout << "Enter date (day.month.year): ";
        cin >> date;
        if (date.size() != 10)
            cout << "Wrong date. Try again\n";
        else break;
    }

    string dateValid = date.substr(6, 4) + date.substr(3, 2) + date.substr(0, 2);

    string currency;
    cout << "Enter currency (USD, EUR, etc.): ";
    cin >> currency;

	string destFile = "file.json";
    DownloadFile(currency, dateValid, destFile);
    
    ifstream read(destFile.c_str());
    if (!read)
    {
        cout << "Error. Can't open the file\n";
        return 1;
    }

    string message;
    char buf[STR_SIZE];
    while (!read.eof())
    {
        read.getline(buf, STR_SIZE);
        message += buf;
    }
    read.close();

    if (FindKey(message, "\"message\"") == "Wrong parameters format")
    {
        cout << "Error. Wrong date or currency value\n";
        return 1;
    }

    string result = FindKey(message, "\"rate\"");
    printf("Date: %s, Currency - %s, 1 unit of currency = %s hryvnia", date.c_str(), currency.c_str(), result.c_str());
}