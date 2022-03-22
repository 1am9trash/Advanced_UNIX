#include "Process.h"

Process::Process(const fs::path &path) {
    this->command = "";
    this->pid = "";
    this->uid = "";
    this->user = "";
    this->get_status(path);
    this->get_fd(path);
}

bool Process::filter(char type, string &argument) {
    if (type == 'c') {
        smatch no_use;
        return regex_search(this->command, no_use, regex(argument));
    }
    return false;
}

string Process::get_name_without_delete(string name) {
    int del_index = name.find(" (deleted)");
    if (del_index != string::npos)
        name.erase(del_index, 11);
    return name;
}

string Process::get_user_from_uid(string uid) {
    passwd* pw;
    pw = getpwuid(atoi(uid.c_str()));
    return string(pw->pw_name);
}

void Process::get_status(const fs::path &path) {
    fs::path status_path = path / "status";

    ifstream fs;
    fs.open(status_path);
    string raw, key, value;
    if (fs.is_open()) {
        while(getline(fs, raw)) {
            stringstream ss;
            ss << raw;
            ss >> key >> value;
            key.pop_back();
            if (key == "Name")
                this->command = value;
            else if (key == "Pid")
                this->pid = value;
            else if (key == "Uid") {
                this->uid = value;
                this->user = this->get_user_from_uid(value);
            }
        }
        fs.close();
    }
}

void Process::get_fd(const fs::path &path) {
    this->get_special_fd(path / "cwd", "cwd");
    this->get_special_fd(path / "root", "rtd");
    this->get_special_fd(path / "exe", "txt");
    this->get_memory_map_fd(path / "maps");
    this->get_normal_fd(path);
}

void Process::get_normal_fd(const fs::path &path) {
    if (access((path / "fd").c_str(), R_OK)) {
        File file;
        file.fd = "NOFD";
        file.name = (path / "fd").string() + " (opendir: Permission denied)";
        this->files.push_back(file);
    } else {
        for (auto &entry: fs::directory_iterator(path / "fd")) {
            File file;
            fs::path folder = entry.path().filename();
            fs::path fd_path = path / "fd" / folder;
            fs::path fdinfo_path = path / "fdinfo" / folder;
            if (!access(fd_path.c_str(), R_OK)) {
                file.type = this->get_fd_type(fd_path);
                file.node = this->get_fd_inode(fd_path);
                this->inode_set.insert(file.node);
                file.name = this->get_name_without_delete(fs::read_symlink(fd_path).string());
            }
            if (!access(fdinfo_path.c_str(), R_OK)) {
                ifstream fs;
                fs.open(fdinfo_path);
                string raw, key, value;
                if (fs.is_open()) {
                    while(getline(fs, raw)) {
                        stringstream ss;
                        ss << raw;
                        ss >> key >> value;
                        key.pop_back();
                        if (key == "flags") {
                            if (value.back() == '0')
                                file.fd = folder.string() + "r";
                            else if (value.back() == '1')
                                file.fd = folder.string() + "w";
                            else
                                file.fd = folder.string() + "u";
                            break;
                        }
                    }
                }
                fs.close();
            } else
                file.fd = "NOFD";
            this->files.push_back(file);
        }
    }
}

void Process::get_memory_map_fd(const fs::path &path) {
    if (!access(path.c_str(), R_OK)) {
        ifstream fs;
        fs.open(path);
        string raw, buf;
        if (fs.is_open()) {
            while(getline(fs, raw)) {
                vector<string> words;
                stringstream ss;
                ss << raw;
                while (ss >> buf)
                    words.push_back(buf);

                if (words.size() == 6) {
                    if (words[4] == "0" || this->inode_set.count(words[4]) == 1)
                        continue;
                    this->inode_set.insert(words[4]);

                    File file;
                    if (words[5].find("( deleted)") != string::npos) {
                        file.type = "unknown";
                        file.fd = "DEL";
                    } else {
                        file.type = "REG";
                        file.fd = "mem";
                    }
                    file.node = words[4];
                    file.name = this->get_name_without_delete(words[5]);
                    this->files.push_back(file);
                }
            }
        }
        fs.close();
    }
}

void Process::get_special_fd(const fs::path &path, string special_fd) {
    File file;
    file.fd = special_fd;
    if (!access(path.c_str(), R_OK)) {
        file.type = this->get_fd_type(path);
        file.node = this->get_fd_inode(path);
        this->inode_set.insert(file.node);
        file.name = this->get_name_without_delete(fs::read_symlink(path).string());
    } else {
        file.type = "unknown";
        file.name = path.string() + " (Permission denied)";
    }
    this->files.push_back(file);
}

string Process::get_fd_type(const fs::path &path) {
    return fs::is_directory(path) ? "DIR" :
        fs::is_regular_file(path) ? "REG" :
        fs::is_character_file(path) ? "CHR" :
        fs::is_fifo(path) ? "FIFO" :
        fs::is_socket(path) ? "SOCK" : "unknown";
}

string Process::get_fd_inode(const fs::path &path) {
    struct stat file_stat;  
    if (stat(path.c_str(), &file_stat) >= 0)
        return to_string(file_stat.st_ino);
    else
        return "";
}

bool Process::operator<(const Process &other) const {
    return atoi(this->pid.c_str()) < atoi(other.pid.c_str());
}

ostream &operator<<(ostream &os, const Process &process) {
    os << left << setw(16) << process.command;
    os << left << setw(16) << process.pid;
    os << left << setw(16) << process.user;
    return os;
}
