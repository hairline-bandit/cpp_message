#ifndef ENCRYPT_DECRYPT_H
#define ENCRYPT_DECRYPT_H

#include <string>

class EncryptDecrypt {
    public:
        EncryptDecrypt(std::string, std::string, char);
        std::string encrypt();
    private:
        unsigned char _key[8][8];
        unsigned char _message[8][8];
        char _mode;
};

#endif