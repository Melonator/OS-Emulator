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
        Screen(std::string name, unsigned int maxLine, size_t memoryRequired);
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
        size_t getMemoryRequired() const;
        void setMemLoc(void* memLoc);
        void* getMemLoc() const;
    private:
        std::string name;
        std::string timestamp;
        unsigned int currLine;
        unsigned int maxLine;
        bool isVisible;
        ProcessState state;
        int currCore;
        size_t memoryRequired;
        void *memLoc;
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
