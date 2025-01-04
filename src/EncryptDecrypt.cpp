#include "EncryptDecrypt.h"
#include <string>
#include <cmath>
#include <iostream>
using namespace std;

const string HEXCHARS = "0123456789abcdef";

EncryptDecrypt::EncryptDecrypt(string message, string key, char mode) {
    for (size_t i = 0; i < key.size(); i += 2) {
        _key[i/16][(i/2)%8] = HEXCHARS.find(key.at(i+1)) + HEXCHARS.find(key.at(i))*16;
    } // read data into a 2d array
    if (mode == 'e') {
        for (int i = 0; i < 64; ++i) {
            if (((unsigned int) i) < message.size()) {
                _message[i/8][i%8] = message.at(i);
            } else {
                _message[i/8][i%8] = 0;
            }
        }
    } else {
        for (size_t i = 0; i < message.size(); i += 2) {
            _message[i/16][(i/2)%8] = HEXCHARS.find(message.at(i+1)) + HEXCHARS.find(message.at(i))*16;
        }
    }
    _mode = mode;
}

// encrypt function acts both as an encryptor and decryptor. as long as the round count and key are correct, it wil decrypt it, too.

string EncryptDecrypt::encrypt() {
    // combine message and key
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            _message[i][j] ^= _key[i][j];
        }
    }
    //round function
    for (int round = 0; round < 69; ++round) {
        // modify message
        for (int i = 0; i < 8; ++i) {
            for (int j = 0; j < 8; ++j) {
                // MSNLSN -> (MSN^LSN)(MSN). the most significant nibble becomes the xor of most and least and the least significant nibble becomes the most significant from before
                _message[i][j] = (((_message[i][j] & 0xF0) ^ ((_message[i][j] << 4) & 0xF0))) + (((_message[i][j] & 0xF0) >> 4) & 0x0F);
            }
        }
        // stop
        // modify key
        for (int i = 0; i < 8; ++i) {
            for (int j = 0; j < 8; ++j) {
                // rotate bytes according to which column they're in (right rotation)
                if (j != 0) {
                    _key[i][j] = (_key[i][j] >> j) + (((_key[i][j] & int(pow(2, j)-1))<<(8-j)) & ~int(pow(2, (8-j))-1));
                }
            }
        }
        for (int i = 0; i < 8; ++i) {
            // rotate rows according to which row they are (right rotation)
            unsigned char tmp[8];
            for (int j = 0; j < 8; ++j) {
                tmp[(j+i)%8] = _key[i][j];
            }
            for (int j = 0; j < 8; ++j) {
                _key[i][j] = tmp[j];
            }
        }
        for (int i = 0; i < 8; ++i) {
            // rotate columns according to which column they are (downwards)
            unsigned char tmp[8];
            for (int j = 0; j < 8; ++j) {
                tmp[(j+i)%8] = _key[j][i];
            }
            for (int j = 0; j < 8; ++j) {
                _key[j][i] = tmp[j];
            }
        }
        // stop
        // combine modified message and key
        for (int i = 0; i < 8; ++i) {
            for (int j = 0; j < 8; ++j) {
                _message[i][j] ^= _key[i][j];
            }
        }
    }
    if (_mode == 'e') {
        string out = "";
        for (int i = 0; i < 8; ++i) {
            for (int j = 0; j < 8; ++j) {
                out += string(1, HEXCHARS.at(_message[i][j]/16));
                out += string(1, HEXCHARS.at(_message[i][j]%16));
            }
        }
        return out;
    } else {
        string out = "";
        for (int i = 0; i < 8; ++i) {
            for (int j = 0; j < 8; ++j) {
                if (_message[i][j] != 0) {
                    out += string(1, _message[i][j]);
                }
            }
        }
        return out;
    }
}   
