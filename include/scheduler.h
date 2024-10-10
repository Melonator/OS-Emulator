#ifndef SCHEDULER_H
#define SCHEDULER_H
#include "cpu.h"
#include "screen.h"

namespace scheduler {
    class Scheduler {
    private:
        cpu::CPU cpu;
        std::vector<std::shared_ptr<screen::Screen>>* ready;
    public:
        Scheduler();
        void run();
        void addProcess(const std::shared_ptr<screen::Screen> &process);
    };
}
#endif //SCHEDULER_H
