#include <iostream>
#include <vector>
#include <sstream>
#include <thread>
#include <mutex>
#include <condition_variable>

#include "../include/color.hpp"
#include "../include/screen.h"
#include "../include/scheduler.h"

bool is_initialized = false; // make false later
bool isMainInputActive = true; // Controls if the main input is active
bool sameScreen = false;
std::mutex mtx;
std::condition_variable cv;

void run(std::vector<std::shared_ptr<screen::Screen>>* processes);
void display();
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

void processThread(const std::string &name, std::vector<std::shared_ptr<screen::Screen>>* processes, const std::shared_ptr<scheduler::Scheduler>& sched) {

    isMainInputActive = false; // Disable main input
    cv.notify_all();

    unsigned int ins = sched->getMinIns() + (rand() % (sched->getMaxIns() - sched->getMinIns() + 1));
    std::shared_ptr<screen::Screen> p = std::make_shared<screen::Screen>(name, ins);
    processes->push_back(p);
    {
        std::lock_guard<std::mutex> readyLock(sched->readyMutex);
        sched->addProcess(p);
    }
    p->show();
    p->run();

    isMainInputActive = true; // Re-enable main input
    cv.notify_all();
}

void reattachThread(std::shared_ptr<screen::Screen> p) {
    isMainInputActive = false;
    cv.notify_all();
    p->show();

    // std::this_thread::sleep_for(std::chrono::milliseconds(500));
    // isMainInputActive = true;
    // cv.notify_all();
}

void schedulerThread(std::shared_ptr<scheduler::Scheduler>& sched) {
    sched->run();
}

void ProcessCommand(std::string const& command, const std::vector<std::string>&  args, std::vector<std::shared_ptr<screen::Screen>>* processes, std::shared_ptr<scheduler::Scheduler>& sched) {
    if (command == "exit") {
        exit(0);
    }
    if (command == "clear") {
        sameScreen = true;
        display();
        return;
    }
    if(command == "initialize") {
        if(is_initialized) {
            std::cout << dye::red("Already initialized\n");
            return;
        }
        sched = std::make_shared<scheduler::Scheduler>(processes);
        // 10 processes
        // for (int i = 0; i < 10; i++) {
        //     std::shared_ptr<screen::Screen> p = std::make_shared<screen::Screen>("screen_" + std::to_string(i));
        //     processes->push_back(p);
        //     sched->addProcess(p);
        // }
        std::thread t(schedulerThread, std::ref(sched));
        t.detach();
        is_initialized = true;
        sameScreen = true;
        std::cout << "Emulator initialized\n";
        return;
    }

    if(!is_initialized) {
        std::cout << dye::red("Please run the 'initialize' command first\n");
        sameScreen = true;
        return;
    }

    if(command == "screen") {
        // std::cout << "screen command recognized. Doing something.\n";
        if(args.size() > 0) {
            if (args.at(0) == "-s") {
                // new screen
                if (args.size() > 2) {
                    std::cout << dye::red("Too many arguments\n");
                    sameScreen = true;
                    return;
                }
                std::string name = args.at(1);
                for (int i = 0; i < processes->size(); i++) {
                    if (processes->at(i)->getName() == name) {
                        std::cout << dye::red("Screen already exists\n");
                        sameScreen = true;
                        // mtx.unlock();
                        // if (mtx.try_lock()) {
                        //     mtx.lock();
                        // }
                        return;
                    }
                }

                // const time_t timestamp = time(NULL);
                // struct tm datetime = *localtime(&timestamp);
                // char output[50] = "";
                // strftime(output, 50, "%m/%d/%Y, %I:%M:%S %p", &datetime);
                // std::cout << output;
                // const std::string timestampStr = output;
                sameScreen = false;
                std::thread t(processThread, name, processes, sched);
                t.detach();

            } else if (args.at(0) == "-r") {
                // reattach
                if (args.size() > 2) {
                    std::cout << dye::red("Too many arguments\n");
                    sameScreen = true;
                } else {
                    std::string name = args.at(1);
                    bool found = false;
                    for (int i = 0; i < processes->size(); i++) {
                        if (processes->at(i)->getName() == name) {
                            // do stuff
                            found = true;
                            isMainInputActive = false; // disable main input
                            sameScreen = false;
                            cv.notify_all();

                            std::thread t(reattachThread, processes->at(i));
                            t.detach();
                            // processes->at(i)->show();

                            break;
                        }
                    }
                    if (!found) {
                        std::cout << dye::red("Screen not found\n");
                        // mtx.unlock();
                        sameScreen = true;
                    }
                }
            } else if (args.at(0) == "-ls") {
                // list all
                if (args.size() > 1) {
                    std::cout << dye::red("Too many arguments\n");
                    sameScreen = true;
                } else {
                    sched->printList();
                    sameScreen = true;
                }
            } else {
                std::cout << dye::red("Invalid argument\n");
                sameScreen = true;
            }
        }
        else {
            std::cout << dye::red("No arguments detected\n");
            sameScreen = true;
        }
    }
    else if (command == "scheduler-test") {
        std::cout << "Starting test.\n";
        sched->startTest();
        sameScreen = true;
    }
    else if (command == "scheduler-stop") {
        std::cout << "Stopping test.\n";
        sched->endTest();
        sameScreen = true;
    }
    else if (command == "report-util") {
        sched->saveList();
        std::cout << "Report generated to csopesy-log.txt\n";
    }
}

std::string GetCommand() {
    std::string input = "";

    std::cout << "Enter a command: ";
    std::getline(std::cin, input);

    return input;
}

void display() {
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
}

void Listen(std::vector<std::shared_ptr<screen::Screen>>* processes, std::shared_ptr<scheduler::Scheduler>& sched) {
    std::string input = "";
    std::string response = "";
    std::string command = "";
    std::vector<std::string> args;
    // while (true) {
        if (!mtx.try_lock()) {
            // std::cout << "Mutex is already locked!\n";
            mtx.unlock();
            // break; // Or add a break to avoid infinite loop
        }
    if (!sameScreen) {
        std::unique_lock<std::mutex> lock(mtx);
        display();
    }
    // cv.wait(lock, [] { return isMainInputActive; });
        // mtx.unlock(); // Explicit unlock for try_lock case

        input = GetCommand();
        ParseCommand(command, args, input); //get command and its arguments
        if(!IsValidCommand(command)) {
            std::cout << "Unknown Command\n\n";
            isMainInputActive = true; // Re-enable main input
            sameScreen = true;
            cv.notify_all();
            return;
            // cv.notify_all();
        }
        ProcessCommand(command, args, processes, sched);
        args.clear();
        std::cout << "\n";
    // }
}

void run(std::vector<std::shared_ptr<screen::Screen>>* processes, std::shared_ptr<scheduler::Scheduler>& sched) {
    std::unique_lock<std::mutex> lock(mtx);

    while (true) {
        if (sameScreen || !isMainInputActive)
            cv.wait(lock, [] { return isMainInputActive; }); // Wait until main input is active
        else if (!sameScreen && isMainInputActive) {
            display();
            cv.wait(lock, [] { return isMainInputActive; });
        }
        // Start listening for input commands
        Listen(processes, sched);
    }
}


int main() {
    // system("cls");
    std::string input = "";
    srand(time(nullptr));
    std::vector<std::shared_ptr<screen::Screen>>* processes = new std::vector<std::shared_ptr<screen::Screen>>();
    std::shared_ptr<scheduler::Scheduler> sched;
    run(processes, sched);
}
