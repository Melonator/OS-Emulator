#ifndef SCHEDULER_H
#define SCHEDULER_H
#include "cpu.h"
#include "screen.h"

namespace scheduler {
    class Scheduler {
    private:
        cpu::CPU cpu;
        std::vector<std::shared_ptr<screen::Screen>>* processes;
        std::vector<std::shared_ptr<screen::Screen>>* ready;
        std::vector<std::shared_ptr<screen::Screen>>* running;
        std::vector<std::shared_ptr<screen::Screen>>* finished;
        int currCycle;
        int processFreq;
        int minIns;
        int maxIns;
        int delay;
        int processIndex;
        bool generateProcess;

    public:
        Scheduler();
        Scheduler(std::vector<std::shared_ptr<screen::Screen>>* processes);
        void run();
        void addProcess(const std::shared_ptr<screen::Screen> &process);
        void addFinished(std::shared_ptr<screen::Screen> s);
        void addRunning(std::shared_ptr<screen::Screen> s);
        void printList();
        void startTest();
        void endTest();
    };
}
#endif //SCHEDULER_H
