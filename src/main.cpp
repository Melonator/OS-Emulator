#include <iostream>
#include <vector>
#include <sstream>
#include "../include/color.hpp"

bool is_initialized = false;

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

void ProcessCommand(std::string const& command, const std::vector<std::string>&  args) {
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
        if(args.size() > 0) {
            std::cout << dye::aqua("Arguments detected\n");
        }

        std::cout << "screen command recognized. Doing something.\n";
    }
    if (command == "scheduler-test") {
        std::cout << "scheduler-test command recognized. Doing something.\n";
    }
    if (command == "scheduler-stop") {
        std::cout << "scheduler-stop command recognized. Doing something.\n";
    }
    if (command == "report-util") {
        std::cout << "report-util command recognized. Doing something.\n";
    }
}

std::string GetCommand() {
    std::string input = "";

    std::cout << "Enter a command: ";
    std::getline(std::cin, input);

    return input;
}

void Listen() {
    std::string input = "";
    std::string response = "";
    std::string command = "";
    std::vector<std::string> args;
    while (input != "clear") {
        input = GetCommand();
        ParseCommand(command, args, input); //get command and its arguments
        if(!IsValidCommand(command)) {
            std::cout << dye::red("Unknown Command\n\n");
            continue;
        }
        ProcessCommand(command, args);
        args.clear();
        std::cout << "\n";
    }
}

int main() {
    system("cls");
    std::string input = "";
    while (true) {
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
        Listen();
    }
}
