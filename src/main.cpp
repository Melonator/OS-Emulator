#include <iostream>
#include <vector>
#include <sstream>
#include <ctime>
#include <thread>
#include <mutex>
#include <condition_variable>

#include "../include/color.hpp"
#include "../include/process.h"
bool is_initialized = true; // make false later
bool isMainInputActive = true; // Controls if the main input is active
std::mutex mtx;
std::condition_variable cv;

void run(std::vector<process::Process>* processes);
void ParseCommand(std::string& command, std::vector<std::string>& args, const std::string& input) {
    if (input.find(' ') == std::string::npos) {
        command = input;
    } else {
        command = input.substr(0, input.find(' '));
    }

    if(input.find(" ") == std::string::npos) {
        return;
    }

    std::stringstream stream(input.substr(input.find(" ") + 1));
    std::string word;
    while (stream >> word) {
        args.push_back(word);
    }
}

bool IsValidCommand(std::string const& command) {
    std::string const valid_commands[7] = {"exit", "clear", "initialize", "screen", "scheduler-test", "scheduler-stop", "report-util"};

    for(auto c: valid_commands) {
        if(c == command) {
            return true;
        }
    }

    return false;
}

void processThread(const std::string &name, const std::string &timestamp, std::vector<process::Process>* processes) {

    isMainInputActive = false; // Disable main input
    cv.notify_all();

    process::Process p(name, timestamp);
    processes->push_back(p);
    p.show();

    isMainInputActive = true; // Re-enable main input
    cv.notify_all();
}

void ProcessCommand(std::string const& command, const std::vector<std::string>&  args, std::vector<process::Process>* processes) {
    if (command == "exit") {
        exit(0);
    }
    if (command == "clear") {
        return;
    }
    if(command == "initialize") {

        if(is_initialized) {
            std::cout << dye::red("Already initialized\n");
            return;
        }
        std::cout << "Initialize command recognized. Doing something.\n";
        is_initialized = true;
        return;
    }

    if(!is_initialized) {
        std::cout << dye::red("Please run the 'initialize' command first\n");
        return;
    }

    if(command == "screen") {
        std::cout << "screen command recognized. Doing something.\n";
        if(args.size() > 0) {
            if (args.at(0) == "-s") {
                // new screen
                if (args.size() > 2) {
                    std::cout << dye::red("Too many arguments\n");
                    return;
                }
                std::string name = args.at(1);
                for (int i = 0; i < processes->size(); i++) {
                    if (processes->at(i).getName() == name) {
                        std::cout << dye::red("Screen already exists\n");
                        mtx.unlock();
                        // if (mtx.try_lock()) {
                        //     mtx.lock();
                        // }
                        return;
                    }
                }

                const time_t timestamp = time(NULL);
                struct tm datetime = *localtime(&timestamp);
                char output[50];
                strftime(output, 50, "%m/%d/%G, %H:%M:%S %p", &datetime);
                std::cout << output;
                // fix strftime output currently broken
                for (int i = 0; i < 49; i++) {
                    if (output[i] == '\0')
                        output[i] = ' ';
                }
                const std::string timestampStr = output;

                std::thread t(processThread, name, timestampStr, processes);
                t.detach();
            } else if (args.at(0) == "-r") {
                // reattach
                if (args.size() > 2) {
                    std::cout << dye::red("Too many arguments\n");
                } else {
                    std::string name = args.at(1);
                    for (int i = 0; i < processes->size(); i++) {
                        if (processes->at(i).getName() == name) {
                            // do stuff
                            isMainInputActive = false; // disable main input
                            cv.notify_all();
                            processes->at(i).show();
                            break;
                        }
                    }
                }
            } else if (args.at(0) == "-ls") {
                // list all
            } else {
                std::cout << dye::red("Invalid argument\n");
            }
        }
        else {
            std::cout << dye::red("No arguments detected\n");
        }
    }
    else if (command == "scheduler-test") {
        std::cout << "scheduler-test command recognized. Doing something.\n";
    }
    else if (command == "scheduler-stop") {
        std::cout << "scheduler-stop command recognized. Doing something.\n";
    }
    else if (command == "report-util") {
        std::cout << "report-util command recognized. Doing something.\n";
    }
}

std::string GetCommand() {
    std::string input = "";

    std::cout << "Enter a command: ";
    std::getline(std::cin, input);

    return input;
}

void Listen(std::vector<process::Process>* processes) {
    std::string input = "";
    std::string response = "";
    std::string command = "";
    std::vector<std::string> args;
    while (true) {
        if (!mtx.try_lock()) {
            std::cout << "Mutex is already locked!\n";
            mtx.unlock();
            // break; // Or add a break to avoid infinite loop
        }
        std::cout << "Mutex acquired by main thread\n";
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [] { return isMainInputActive; });
        // mtx.unlock(); // Explicit unlock for try_lock case

        std::cout << "Main thread input unlocked\n";
        input = GetCommand();
        ParseCommand(command, args, input); //get command and its arguments
        if(!IsValidCommand(command)) {
            std::cout << "Unknown Command\n\n";
            isMainInputActive = true; // Re-enable main input
            cv.notify_all();
            // cv.notify_all();
            continue;
        }
        ProcessCommand(command, args, processes);
        args.clear();
        std::cout << "\n";
        if (mtx.try_lock()) {
            std::cout << "Pumasok dito\n";
            continue;
            // mtx.unlock();
        }
        if (!mtx.try_lock()) {
            mtx.unlock();
        }
    }
}

void run(std::vector<process::Process>* processes) {
    while (true) {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [] { return isMainInputActive; }); // Wait until main input is active

        system("cls");
        std::cout <<
                R"( _____  _____  ___________ _____ _______   __
/  __ \/  ___||  _  | ___ \  ___/  ___\ \ / /
| /  \/\ `--. | | | | |_/ / |__ \ `--. \ V /
| |     `--. \| | | |  __/|  __| `--. \ \ /
| \__/\/\__/ /\ \_/ / |   | |___/\__/ / | |
 \____/\____/  \___/\_|   \____/\____/  \_/

)";

        std::cout << dye::green("Hello, Welcome to the CSOPESY commandline!\n");
        std::cout << dye::yellow("Type 'exit' to quit, 'clear' to clear the screen\n");

        // Start listening for input commands
        Listen(processes);
    }
}


int main() {
    time_t timestamp = time(NULL);
    system("cls");
    std::string input = "";
    std::vector<process::Process>* processes = new std::vector<process::Process>();
    run(processes);
}
