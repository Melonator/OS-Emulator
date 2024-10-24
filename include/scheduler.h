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
        unsigned int processFreq; // The frequency of generating processes in the "scheduler-test" command in CPU cycles. The range is [1, 2^32]. If one, a new process is generated at the end of each CPU cycle.
        unsigned int minIns; // The minimum instructions/command per process. The range is [1, 2^32].
        unsigned int maxIns; // The maximum instructions/command per process. The range is [1, 2^32].
        unsigned int delay; // Delay before executing the next instruction in CPU cycles. The delay is a "busy-waiting" scheme wherein the process remains in the CPU. The range is [0, 2^32]. If zero, each instruction is executed per CPU cycle.
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
