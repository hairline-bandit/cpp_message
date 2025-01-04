#include "Message.h"
#include "EncryptDecrypt.h"
#include <regex>
#include <string>

using namespace std;

Message::Message(const string MSG, const string FRM, const string T, const string TSTAMP) { // constructor for if we send a message (just DATA)
    _message = MSG;
    _from = FRM;
    _to = T;
    _timestamp = TSTAMP;
}

Message::Message(const string DATA) { // constructor for if we receive a message from the update() function
    _from = DATA.substr(DATA.find(".from")+10, DATA.substr(DATA.find(".from")+10, DATA.size()-1).find("\\"));
    _to = DATA.substr(DATA.find(".to")+8, DATA.substr(DATA.find(".to")+8, DATA.size()-1).find("\\"));
    _timestamp = DATA.substr(DATA.find(".time")+10, DATA.substr(DATA.find(".time")+10, DATA.size()-1).find("\\"));
    _message = DATA.substr(DATA.find(".message")+13, DATA.substr(DATA.find(".message")+13, DATA.size()-1).find("\\\"}"));
}

Message::Message(const string DATA, const string KEY) { // constructor for if we have to decrypt 
    string curr = "";
    int counter = 0;
    for (size_t i = 0; i < DATA.size(); ++i) {
        if (DATA.at(i) != ':') {
            curr += string(1, DATA.at(i));
        } else {
            EncryptDecrypt* decryptor = new EncryptDecrypt(curr, KEY, 'd');
            if (counter == 0) {
                _message = decryptor->encrypt();
            } else if (counter == 1) {
                _from = decryptor->encrypt();
            } else if (counter == 2) {
                _to = decryptor->encrypt();
            } else {
                _timestamp = decryptor->encrypt();
            }
            delete decryptor;
            decryptor = nullptr;
            curr = "";
            counter++;
        }
    }
}

string Message::getMessage() const {
    return _message;
}

string Message::getFrom() const {
    return _from;
}

string Message::getTo() const {
    return _to;
}

string Message::getTimestamp() const {
    return _timestamp;
}
