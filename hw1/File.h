#pragma once

#include <regex>
#include <string>
#include <fstream>
#include <iomanip>
using namespace std;

class File {
public:
    string fd, type, node, name;

    File();
    bool filter(char type, string &argument);
    friend ostream &operator<<(ostream &os, const File &file);
};
