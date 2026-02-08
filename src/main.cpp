#include <iostream>
#include <string>
#include <cstdlib>
#include <chrono>
#include <ctime>
#include <unordered_map>
# include <vector>
#include <thread>
#include <atomic>
#include <vector>
#include <mutex>

using namespace std;
mutex procMutex;

void printHelp() {
    std::cout << "Available commands:\n"
              << "  help               - Show this message\n"
              << "  echo <text>        - Print your message\n"
              << "  clear              - Clear the screen\n"
              << "  time               - Show system time\n"
              << "  version            - Show OS version\n"
              << "  about              - Developer info\n"
              << "  reboot             - Restart OS_Simulator\n"

              << "\nFile System:\n"
              << "  pwd                - Print working directory\n"
              << "  mkdir <name>       - Create directory\n"
              << "  ls                 - List contents\n"
              << "  cd <dir>           - Change directory\n"
              << "  touch <file>       - Create file\n"
              << "  rm <name>          - Delete file/folder\n"

              << "\nProcess Simulation:\n"
              << "  run <name> <time>  - Create process\n"
              << "  ps                 - List processes\n"
              << "  kill <pid>         - Terminate process\n"

              << "\nScheduler Control:\n"
              << "  start              - Start Round Robin scheduler\n"
              << "  stop               - Stop scheduler\n"

              << "\nSystem:\n"
              << "  exit               - Shut down OS_Simulator\n";
}

struct Directory {
    string name;
    Directory* parent;
    unordered_map<string, Directory*> subdirs;
    unordered_map<string, string> files;
};

string getPath(Directory* dir) {
    if (dir->parent == nullptr) return "/";
    return getPath(dir->parent) + dir->name + "/";
}

struct Process {
    int pid;
    string name;
    int remainingTime;
    string state;
};



int main(){
    cout<<"OS_Simulator booting up...\n";
    cout<<"Type 'help' to see available commands.\n";
    string command;

    Directory* root = new Directory{"/", nullptr};
    Directory* current = root;

    vector<Process> processes;
    int nextPID = 1;
    int quantum = 2;
    int currentIndex = 0;
    atomic<bool> schedulerRunning(true);
    atomic<bool> schedulingEnabled(false);


    thread scheduler([&]() {
    int index = 0;

    while (schedulerRunning) {

        if (!schedulingEnabled) {
            this_thread::sleep_for(chrono::milliseconds(200));
            continue;
        }

        this_thread::sleep_for(chrono::seconds(1));

        lock_guard<mutex> lock(procMutex);  // 👈 LOCK

        if (processes.empty()) continue;

        if (index >= processes.size())
            index = 0;

        Process &p = processes[index];

        if (p.state != "Terminated") {
            p.state = "Running";

        cout << "\n[Scheduler] Running PID "
             << p.pid << " (" << p.name << ")\n";

        p.remainingTime -= quantum;

        if (p.remainingTime <= 0) {
            p.state = "Terminated";
            cout << "[Scheduler] Process "
                 << p.pid << " finished\n";
        } else {
            p.state = "Ready";
        }
        }

    index = (index + 1) % processes.size();
    }

});



    while(true){
        cout << getPath(current) << "> ";

        getline(cin, command);
        if(command == "help"){
            printHelp();
        }
        else if(command.rfind("echo", 0)==0){
            cout<<command.substr(5)<<"\n";
        }
        else if(command == "clear"){
            system("cls");
        }
        else if (command == "time") {
            auto now = chrono::system_clock::now();
            time_t t = chrono::system_clock::to_time_t(now);
            cout << "Current time: " << ctime(&t);
        }
        else if (command == "version") {
            cout << "OS_Simulator Version 1.0.0 (Simulator Build)\n";
        }   
        else if (command == "about") {
            cout << "OS_Simulator - a toy operating system simulation.\n";
            cout << "Created by Sanvi Ranjan.\n";
        }
        else if (command == "reboot") {
            system("cls");   
            cout << "Rebooting TinyOS...\n\n";
            cout << "TinyOS booting up...\n";
            cout << "Type 'help' to see available commands.\n\n";
        }


        else if(command == "exit"){
            cout<<"Shutting down OS_Simulator...\n";
            break;
        }

        else if (command == "pwd") {
            cout << getPath(current) << endl;
        }

        else if (command.rfind("mkdir ", 0) == 0) {
            string dirname = command.substr(6);

            if (dirname.empty()) {
                cout << "Usage: mkdir <name>\n";
                continue;
            }

            if (current->subdirs.count(dirname)) {
                cout << "Directory already exists\n";
            } else {
                Directory* newDir = new Directory{dirname, current};
                current->subdirs[dirname] = newDir;
            }
        }


        else if (command == "ls") {
            for (auto& d : current->subdirs)
                cout << d.first << "/  ";
            for (auto& f : current->files)
                cout << f.first << "  ";
            cout << endl;
        }

        else if (command.rfind("cd ", 0) == 0) {
            string dirname = command.substr(3);

            if (dirname == "..") {
                if (current->parent != nullptr)
                    current = current->parent;
            }
            else if (current->subdirs.count(dirname)) {
                current = current->subdirs[dirname];
            }
            else {
                cout << "Directory not found\n";
            }
        }

        else if (command.rfind("touch ", 0) == 0) {
            string fname = command.substr(6);

            if (current->files.count(fname)) {
                cout << "File already exists\n";
            } else {
                current->files[fname] = "";
            }
        }

        else if (command.rfind("write ", 0) == 0) {
            int space = command.find(' ', 6);
            if (space == string::npos) {
                cout << "Usage: write <file> <text>\n";
                continue;
            }

            string fname = command.substr(6, space-6);
            string text = command.substr(space+1);

            if (current->files.count(fname)) {
                current->files[fname] = text;
            } else {
            cout << "File not found\n";
            }
        }

        else if (command.rfind("read ", 0) == 0) {
            string fname = command.substr(5);

            if (current->files.count(fname)) {
                cout << current->files[fname] << endl;
            } else {
            cout << "File not found\n";
            }
        }

        else if (command.rfind("run ", 0) == 0) {
            string pname = command.substr(4);

            Process p;
            p.pid = nextPID++;
            p.name = pname;
            p.remainingTime = rand()%15 + 5; // random 5–20
            p.state = "Ready";

            processes.push_back(p);

            cout << "Process " << pname
            << " created (PID " << p.pid << ")\n";
        }

        else if (command == "start") {
            schedulingEnabled = true;
            cout << "Scheduler started\n";
        }

        else if (command == "stop") {
            schedulingEnabled = false;
            cout << "Scheduler paused\n";
        }

        else if (command == "ps") {
            cout << "PID\tName\tTime\tState\n";

            for (auto &p : processes) {
                cout << p.pid << "\t"
                << p.name << "\t"
                << p.remainingTime << "\t"
                << p.state << "\n";
            }
        }

        else if (command.rfind("kill ", 0) == 0) {

            int pid = stoi(command.substr(5));

            lock_guard<mutex> lock(procMutex);  // 👈 LOCK

            bool found = false;

            for (auto &p : processes) {
                if (p.pid == pid && p.state != "Terminated") {
                    p.state = "Terminated";
                    cout << "Process " << pid << " killed\n";
                    found = true;
                    break;
                }
            }

            if (!found)
            cout << "PID not found\n";
        }



        else if(!command.empty()){
            cout<<"Unknown command. Type 'help' for a list.\n";
        }
    }
   
    return 0;
}