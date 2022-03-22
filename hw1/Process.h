#pragma once

#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <sys/stat.h> 
#include <unistd.h>
#include <fcntl.h> 
#include <pwd.h>
#include <set>
#include <map>
#include <regex>
#include <fstream>
#include <filesystem>

#include "File.h"
using namespace std;
namespace fs = filesystem;

class Process {
public:
    string command, pid, uid, user;
    vector<File> files;
    set<string> inode_set;

    Process();
    Process(const fs::path &path);
    bool filter(char type, string & argument);
    string get_user_from_uid(string uid);
    string get_name_without_delete(string name);
    void get_status(const fs::path &path);
    void get_fd(const fs::path &path);
    void get_normal_fd(const fs::path &path);
    void get_memory_map_fd(const fs::path &path);
    void get_special_fd(const fs::path &path, string fd_type);
    string get_fd_type(const fs::path &path);
    string get_fd_inode(const fs::path &path);
    bool operator<(const Process &other) const;
    friend ostream &operator<<(ostream &os, const Process &process);
};
