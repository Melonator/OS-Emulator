#include "../include/screen.h"

#include <condition_variable>
#include <iostream>
#include <mutex>
#include <ctime>
#include <fstream>
using namespace screen;

    // External variables for controlling input
    extern bool isMainInputActive;
    extern std::mutex mtx;
    extern std::condition_variable cv;

    std::string timestampFormat();

    Screen::Screen() {
        this->name = "";
        this->timestamp = "";
        this->currLine = 0;
        this->maxLine = 0;
        this->state = ProcessState::READY;
        this->currCore = -1;
        this->startTime = "";
        this->endTime = "";
        this->isVisible = false;
    }
    Screen::Screen(const std::string name) {
        this->name = name;
        this->timestamp = timestampFormat();
        this->currLine = 0;
        this->maxLine = 100;
        this->state = ProcessState::READY;
        this->currCore = -1;
        this->isVisible = true;
    }
    std::string Screen::getName() {
        return this->name;
    }

    void Screen::hide() {
        this->isVisible = false;
    }

    void Screen::show() {
        this->isVisible = true;
        // system("cls");
        this->run();
        // system("cls");
    }

    void Screen::processInfo() const {
        std::cout << "Process: " << this->name << "\n";
        std::cout << "Current instruction line: " << this->currLine << " / " << this->maxLine << "\n";
        std::cout << "Timestamp: " << this->timestamp << "\n";
    }

    void Screen::run() {
        system("cls");
        system("cls");
        while (true) {
            if (isVisible) {
                processInfo();
                this->Listen();
                // break; // Exit loop after listening to detach
            }
            else {
                break;
            }
            // currLine++;
            // std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }

    std::string Screen::GetCommand() {
        std::string input = "";

        std::cout << "Enter a command: ";
        std::getline(std::cin, input);

        return input;
    }

    void Screen::ParseCommand(std::string& command, std::vector<std::string>& args, std::string input) {
        std::string temp = "";
        for (int i = 0; i < input.length(); i++) {
            if (input[i] == ' ') {
                command = temp;
                temp = "";
            } else {
                temp += input[i];
            }
        }
        command = temp;
        temp = "";
        for (int i = 0; i < input.length(); i++) {
            if (input[i] == ' ') {
                args.push_back(temp);
                temp = "";
            } else {
                temp += input[i];
            }
        }
        args.push_back(temp);
    }

    bool Screen::IsValidCommand(std::string command) {
        if (command == "exit" || command == "process-smi") {
            return true;
        }
        return false;
    }

    void Screen::ProcessCommand(std::string const& command, const std::vector<std::string>&  args) {
        if (command == "exit") {
            std::cout << "Exit screen " << this->getName() << "\n";
            this->hide();
            // if (!mtx.try_lock()) {
                mtx.unlock();
            // }
            std::unique_lock<std::mutex> lock(mtx);
            isMainInputActive = true; // Re-enable main input
            cv.notify_all(); // Notify the main thread
        }
        else if (command == "process-smi") {
            // std::cout << "Process SMI command recognized. Doing something.\n";
        }
    }
    void Screen::Listen() {
        std::string input = "";
        std::string response = "";
        std::string command = "";
        std::vector<std::string> args;
        while (!this->IsValidCommand(command)) {
            input = this->GetCommand();
            this->ParseCommand(command, args, input); //get command and its arguments
            if(!this->IsValidCommand(command)) {
                std::cout << "Unknown Command\n\n";
            }
            else {
                this->ProcessCommand(command, args);
                args.clear();
                std::cout << "\n";
            }
        }
    }

    void Screen::print() {
        // output to log file
        std::ofstream logFile;
        std::string fileName = name + "_log.txt";
        logFile.open(fileName, std::ios_base::app);
        state = ProcessState::RUNNING;

        // perform print command for n amount of times
        for (int i = currLine; i < maxLine; i++) {
            std::string timestamp = timestampFormat();
            logFile << "(" << timestamp << ") " << "Core:" << currCore << " \"Hello world from " << name << "!\"\n";
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            currLine++;
        }
        logFile.close();
        state = ProcessState::TERMINATED;
    }

    void Screen::setCore(int core) {
        this->currCore = core;
    }
    std::string timestampFormat() {
        const time_t timestamp = time(NULL);
        struct tm datetime = *localtime(&timestamp);
        char output[50] = "";
        strftime(output, 50, "%m/%d/%Y, %I:%M:%S %p", &datetime);
        const std::string timestampStr = output;
        return timestampStr;
    }

    int Screen::getCurrLine() const {
        return this->currLine;
    }

    int Screen::getMaxLine() const {
        return this->maxLine;
    }

    std::string Screen::toString() {
        std::string stateStr = "";
        if (currCore == -1 && state == ProcessState::TERMINATED)
            stateStr = "Finished";
        else if (currCore == -1 && state == ProcessState::READY)
            stateStr = "Ready";
        else
            stateStr = "Core: " + std::to_string(currCore);
        std::string time = "";
        if (state == ProcessState::RUNNING)
            time = startTime;
        else if (state == ProcessState::TERMINATED)
            time = endTime;
        return name + "   (" + time + ")   " + stateStr + "   " + std::to_string(currLine) + " / " + std::to_string(maxLine);
    }

    void Screen::setStartTime() {
        startTime = timestampFormat();
    }

    void Screen::setEndTime() {
        endTime = timestampFormat();
    }
