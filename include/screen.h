#ifndef PROCESS_H
#define PROCESS_H
#include <string>
#include <vector>
enum class ProcessState {
    READY,
    RUNNING,
    WAITING,
    TERMINATED
};

namespace screen {
    class Screen {
    public:
        Screen(std::string name);
        Screen();
        std::string getName();
        void hide();
        void show();
        void run();
        void print();
        void setCore(int core);
        int getCurrLine() const;
        int getMaxLine() const;
    private:
        std::string name;
        std::string timestamp;
        int currLine;
        int maxLine;
        bool isVisible;
        ProcessState state;
        int currCore;
        std::string GetCommand();
        void ParseCommand(std::string& command, std::vector<std::string>& args, std::string input);
        bool IsValidCommand(std::string command);
        void ProcessCommand(std::string const& command, const std::vector<std::string>&  args);
        void Listen();
    };
}
#endif //PROCESS_H
