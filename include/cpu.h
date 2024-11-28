#ifndef CPU_H
#define CPU_H
#include <memory>
#include <vector>
#include "screen.h"

enum class CoreState {
    IDLE,
    BUSY,
};
namespace cpu {
    class Core {
    private:
        int id{};
        CoreState state;
        unsigned int quantum{};
        unsigned int remainingQuantum{};
        unsigned int currCycle{};
        unsigned int delay{};
        bool cycleFinished{};
        std::shared_ptr<screen::Screen> currScreen;
        std::vector<std::shared_ptr<screen::Screen>>* ready{};
        std::vector<std::shared_ptr<screen::Screen>>* running{};
        std::vector<std::shared_ptr<screen::Screen>>* finished{};
        std::mutex* readyMutex;
        std::mutex* runningMutex;
        std::mutex* finishedMutex;
        size_t idleTicks = 0;
        size_t activeTicks = 0;
        std::mutex totalMutex;
        std::mutex activeMutex;
        std::mutex idleMutex;
    public:
        Core();
        Core(int id, unsigned int quantum, unsigned int delay, std::mutex* readyMutex, std::mutex* runningMutex, std::mutex* finishedMutex, std::vector<std::shared_ptr<screen::Screen>>* ready, std::vector<std::shared_ptr<screen::Screen>> *running, std::vector<std::shared_ptr<screen::Screen>> *finished);
        void work();
        CoreState getState() const;
        void setState(CoreState state);
        void setScreen(const std::shared_ptr<screen::Screen> &s);
        void removeRunning(const std::shared_ptr<screen::Screen>& s);
        void addFinished(const std::shared_ptr<screen::Screen>& s);
        void addReady(const std::shared_ptr<screen::Screen>& s);
        bool isCycleFinished() const;
        void setCycleFinished(bool cycleFinished);
        void preempt();
        std::shared_ptr<screen::Screen> getCurrScreen();
        size_t getTotalTicks();
        size_t getIdleTicks();
        size_t getActiveTicks();
    };

    class CPU {
    private:
      std::vector<std::shared_ptr<Core>> cores;
      int numCores;
      unsigned int quantum;
      std::string algorithm;
  public:
        CPU();
        CPU(int numCores, unsigned int quantum, unsigned int delay, std::mutex* readyMutex, std::mutex* runningMutex, std::mutex* finishedMutex, const std::string &algorithm, std::vector<std::shared_ptr<screen::Screen>>* ready, std::vector<std::shared_ptr<screen::Screen>>* running, std::vector<std::shared_ptr<screen::Screen>>* finished);
        int getNumCores() const;
        std::vector<std::shared_ptr<Core>> getCores() const;
        bool allCyclesFinished() const;
        void setAllCyclesFinished(bool cycleFinished);
        float getUtilization();
        int getAvailableCores();
        void work();
        size_t getTotalTicks();
        size_t getIdleTicks();
        size_t getActiveTicks();
    };

}
#endif //CPU_H
