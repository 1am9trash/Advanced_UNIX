#include "File.h"

File::File() {
    this->fd = "";
    this->type = "";
    this->node = "";
    this->name = "";
}

bool File::filter(char type, string &argument) {
    if (type == 'f') {
        smatch no_use;
        return regex_search(this->name, no_use, regex(argument));
    }
    if (type == 't')
        return this->type == argument; 
    return false;
}

ostream &operator<<(ostream &os, const File &file) {
    os << left << setw(16) << file.fd;
    os << left << setw(16) << file.type;
    os << left << setw(20) << file.node;
    os << left << setw(30) << file.name;
    return os;
}
