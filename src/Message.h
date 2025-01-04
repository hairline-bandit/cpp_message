#ifndef MESSAGE_H
#define MESSAGE_H

#include <string>

class Message {
    public: 
        Message(const std::string, const std::string, const std::string, const std::string);
        Message(const std::string);
        Message(const std::string, const std::string);
        std::string getMessage() const;
        std::string getFrom() const;
        std::string getTo() const;
        std::string getTimestamp() const;
    private:
        std::string _message;
        std::string _from;
        std::string _to;
        std::string _timestamp;
};

#endif