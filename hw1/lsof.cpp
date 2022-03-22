#include "Process.h"
using namespace std;

void lsof(map<char, string> &arguments) {
    vector<Process> processes;
    string base = "/proc";
    regex num_re(R"(^/proc/[0-9]+$)");

    for (auto &entry: fs::directory_iterator(base)) {
        if (regex_match(entry.path().c_str(), num_re)) {
            Process p(entry.path());
            processes.push_back(p);
        }
    }

    cout << left << setw(16) << "COMMAND";
    cout << left << setw(16) << "PID";
    cout << left << setw(16) << "USER";
    cout << left << setw(16) << "FD";
    cout << left << setw(16) << "TYPE";
    cout << left << setw(20) << "NODE";
    cout << left << setw(30) << "NAME";
    cout << "\n";
    sort(processes.begin(), processes.end());
    for (Process &process : processes) {
        for (File &file : process.files) {
            if (arguments.count('c') == 1 && !process.filter('c', arguments['c'])) 
                continue;
            if (arguments.count('t') == 1 && !file.filter('t', arguments['t'])) 
                continue;
            if (arguments.count('f') == 1 && !file.filter('f', arguments['f'])) 
                continue;
            cout << process << " " << file << "\n";
        }
    }
}

int main(int argc, char **argv) {
    int opt;
    map<char, string> arguments;
    while((opt = getopt(argc, argv, "c:t:f:")) != -1)
        arguments[opt] = optarg;

    lsof(arguments);

    return 0;
}
