#include "Utilities.h"
#include <vector>
#include <regex>
#include <string>

using namespace std;

vector<string> split(const string &s, const regex &sep_regex) {
    sregex_token_iterator iter(s.begin(), s.end(), sep_regex, -1);
    sregex_token_iterator end;
    return {iter, end};
} // split using regex for delimiter. return a vector of each part