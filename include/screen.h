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
        Screen(std::string name, unsigned int maxLine);
        Screen();
        std::string getName();
        void hide();
        void show();
        void run();
        void print();
        void setCore(int core);
        unsigned int getCurrLine() const;
        unsigned int getMaxLine() const;
        std::string toString();
        void setStartTime();
        void setEndTime();
        void processInfo() const;
        bool isFinished() const;
        void setState(ProcessState state);
    private:
        std::string name;
        std::string timestamp;
        unsigned int currLine;
        unsigned int maxLine;
        bool isVisible;
        ProcessState state;
        int currCore;
        std::string startTime;
        std::string endTime;
        std::string GetCommand();
        void ParseCommand(std::string& command, std::vector<std::string>& args, std::string input);
        bool IsValidCommand(std::string command);
        void ProcessCommand(std::string const& command, const std::vector<std::string>&  args);
        void Listen();
    };
}
#endif //PROCESS_H
