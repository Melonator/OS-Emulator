#ifndef SCHEDULER_H
#define SCHEDULER_H
#include "cpu.h"
#include "memory_allocator.h"
#include "screen.h"

namespace scheduler {
    class Scheduler {
    private:
        cpu::CPU cpu;
        std::vector<std::shared_ptr<screen::Screen>>* processes;
        std::vector<std::shared_ptr<screen::Screen>>* ready;
        std::vector<std::shared_ptr<screen::Screen>>* running;
        std::vector<std::shared_ptr<screen::Screen>>* finished;
        std::shared_ptr<allocator::FlatModel> flatModel;
        std::shared_ptr<allocator::Paging> pagingModel;
        unsigned int currCycle;
        unsigned int processFreq; // The frequency of generating processes in the "scheduler-test" command in CPU cycles. The range is [1, 2^32]. If one, a new process is generated at the end of each CPU cycle.
        unsigned int minIns; // The minimum instructions/command per process. The range is [1, 2^32].
        unsigned int maxIns; // The maximum instructions/command per process. The range is [1, 2^32].
        unsigned int delay; // Delay before executing the next instruction in CPU cycles. The delay is a "busy-waiting" scheme wherein the process remains in the CPU. The range is [0, 2^32]. If zero, each instruction is executed per CPU cycle.
        unsigned int processIndex;
        unsigned int quantum;
        size_t minMemPerProc;
        size_t maxMemPerProc;
        size_t memory;
        size_t blockSize;
        bool generateProcess;
        bool screenLS;
        std::mutex runningMutex;
        std::mutex finishedMutex;
        std::string allocator;
        size_t totalTicks;
        size_t idleTicks;
        size_t activeTicks;
    public:
        std::mutex readyMutex;
        Scheduler();
        Scheduler(std::vector<std::shared_ptr<screen::Screen>>* processes);
        void run();
        void addProcess(const std::shared_ptr<screen::Screen> &process);
        // void addFinished(std::shared_ptr<screen::Screen> s);
        void addRunning(std::shared_ptr<screen::Screen> s);
        void printList();
        void startTest();
        void endTest();
        void saveList();
        std::shared_ptr<screen::Screen> createProcess(std::string name);
        unsigned int getMinIns() const;
        unsigned int getMaxIns() const;
        std::string screenList();
        void setScreenLS();
        std::vector<std::string> getRunningNames();
        void preempt(std::string name);
        bool isRunning(std::string name);
        void setMemLocNull(std::string name);
        void vmStat();
    };
}
#endif //SCHEDULER_H
