#ifndef PROCESS_H
#define PROCESS_H
#include <string>
#include <vector>
namespace process {
    class Process {
    public:
        Process(std::string name, std::string timestamp);
        Process();
        std::string getName();
        void hide();
        void show();

    private:
        std::string name;
        std::string timestamp;
        int currLine;
        bool isVisible;
        void run();

        std::string GetCommand();
        void ParseCommand(std::string& command, std::vector<std::string>& args, std::string input);
        bool IsValidCommand(std::string command);
        void ProcessCommand(std::string const& command, const std::vector<std::string>&  args);
        void Listen();
    };
}
#endif //PROCESS_H
