#ifndef SCHEDULER_H
#define SCHEDULER_H
#include "cpu.h"
#include "screen.h"

namespace scheduler {
    class Scheduler {
    private:
        cpu::CPU cpu;
        std::vector<std::shared_ptr<screen::Screen>>* ready;
        std::vector<std::shared_ptr<screen::Screen>>* running;
        std::vector<std::shared_ptr<screen::Screen>>* finished;
    public:
        Scheduler();
        void run();
        void addProcess(const std::shared_ptr<screen::Screen> &process);
        void addFinished(std::shared_ptr<screen::Screen> s);
        void addRunning(std::shared_ptr<screen::Screen> s);
        void printList();
    };
}
#endif //SCHEDULER_H
