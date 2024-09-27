#include "../include/process.h"

#include <condition_variable>
#include <iostream>
#include <mutex>
using namespace process;

    // External variables for controlling input
    extern bool isMainInputActive;
    extern std::mutex mtx;
    extern std::condition_variable cv;

    Process::Process() {
        this->name = "";
        this->timestamp = "";
        this->currLine = 0;
        this->isVisible = false;
    }
    Process::Process(const std::string name, const std::string timestamp) {
        this->name = name;
        this->timestamp = timestamp;
        this->currLine = 0;
        this->isVisible = true;
    }
    std::string Process::getName() {
        return this->name;
    }

    void Process::hide() {
        this->isVisible = false;
    }

    void Process::show() {
        this->isVisible = true;
        this->run();
    }

    void Process::run() {
        system("cls");
        system("cls");
        while (true) {
            if (this->isVisible) {
                std::cout << "Process: " << this->name << "\n";
                std::cout << "Current instruction line: " << this->currLine << "\n";
                std::cout << "Timestamp: " << this->timestamp << "\n";
                this->Listen();
                break; // Exit loop after listening to detach
            }
        }
    }

    std::string Process::GetCommand() {
        std::string input = "";

        std::cout << "Enter a command: ";
        std::getline(std::cin, input);

        return input;
    }

    void Process::ParseCommand(std::string& command, std::vector<std::string>& args, std::string input) {
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

    bool Process::IsValidCommand(std::string command) {
        if (command == "exit" || command == "process-smi") {
            return true;
        }
        return false;
    }

    void Process::ProcessCommand(std::string const& command, const std::vector<std::string>&  args) {
        if (command == "exit") {
            this->hide();
            // if (!mtx.try_lock()) {
                mtx.unlock();
            // }
            std::unique_lock<std::mutex> lock(mtx);
            isMainInputActive = true; // Re-enable main input
            cv.notify_all(); // Notify the main thread
        }
        else if (command == "process-smi") {
            std::cout << "Process SMI command recognized. Doing something.\n";
        }
    }
    void Process::Listen() {
        std::string input = "";
        std::string response = "";
        std::string command = "";
        std::vector<std::string> args;
        while (input != "clear" && input != "exit") {
            input = this->GetCommand();
            this->ParseCommand(command, args, input); //get command and its arguments
            if(!this->IsValidCommand(command)) {
                std::cout << "Unknown Command\n\n";
                continue;
            }
            this->ProcessCommand(command, args);
            args.clear();
            std::cout << "\n";
        }
    }
