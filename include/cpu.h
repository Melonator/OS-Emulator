#ifndef CPU_H
#define CPU_H
#include <memory>
#include <vector>
#include "screen.h"

enum class CoreState {
    IDLE,
    BUSY
};
namespace cpu {
    class Core {
    private:
        int id;
        CoreState state;
        std::shared_ptr<screen::Screen> currScreen;
        std::vector<std::shared_ptr<screen::Screen>>* running;
        std::vector<std::shared_ptr<screen::Screen>>* finished;
    public:
        Core();
        Core(int id, std::vector<std::shared_ptr<screen::Screen>> *running, std::vector<std::shared_ptr<screen::Screen>> *finished);
        void work();
        CoreState getState() const;
        void setState(CoreState state);
        void setScreen(std::shared_ptr<screen::Screen> s);
        void removeRunning(std::shared_ptr<screen::Screen> s);
        void addFinished(std::shared_ptr<screen::Screen> s);
    };

    class CPU {
    private:
      std::vector<std::shared_ptr<Core>> cores;
      int numCores;
      std::vector<std::shared_ptr<screen::Screen>>* running;
      std::vector<std::shared_ptr<screen::Screen>>* finished;
  public:
        CPU();
        CPU(int numCores, std::vector<std::shared_ptr<screen::Screen>>* running, std::vector<std::shared_ptr<screen::Screen>>* finished);
        int getNumCores() const;
        std::vector<std::shared_ptr<Core>> getCores() const;
    };

}
#endif //CPU_H
