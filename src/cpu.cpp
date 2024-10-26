#include "../include/cpu.h"
#include <iostream>
#include <thread>
using namespace cpu;

void coreThread(const std::shared_ptr<Core> &core) {
    core->work();
}

CPU::CPU() {

}

CPU::CPU(int numCores, unsigned int quantum, unsigned int delay, std::mutex* readyMutex, std::mutex* runningMutex, std::mutex* finishedMutex, const std::string &algorithm, std::vector<std::shared_ptr<screen::Screen>>* ready, std::vector<std::shared_ptr<screen::Screen>>* running, std::vector<std::shared_ptr<screen::Screen>>* finished) {
    //Logic for initializing threads and other stuff
    // std::cout << "I have " << numCores << " core(s)!\n";
    this->numCores = numCores;
    this->algorithm = algorithm;
    this->quantum = quantum;
    for (int i = 0; i < numCores; i++) {
        std::shared_ptr<Core> core = std::make_shared<Core>(i, quantum, delay, readyMutex, runningMutex, finishedMutex, ready, running, finished);
        std::thread t(coreThread, core);
        t.detach();
        this->cores.push_back(core);
    }
}

int CPU::getNumCores() const {
    return this->numCores;
}

std::vector<std::shared_ptr<Core>> CPU::getCores() const {
    return this->cores;
}

bool CPU::allCyclesFinished() const {
    for (int i = 0; i < numCores; i++) {
        if (!cores.at(i)->isCycleFinished() && cores.at(i)->getState() != CoreState::IDLE)
            return false;
    }
    return true;
}

void CPU::setAllCyclesFinished(bool cycleFinished) {
    for (int i = 0; i < numCores; i++) {
        cores.at(i)->setCycleFinished(cycleFinished);
    }
}

float CPU::getUtilization() {
    int busy = 0;
    for (int i = 0; i < numCores; i++) {
        if (cores.at(i)->getState() == CoreState::BUSY)
            busy++;
    }
    return (float)busy / numCores;
}

int CPU::getAvailableCores() {
    int available = 0;
    for (int i = 0; i < numCores; i++) {
        if (cores.at(i)->getState() == CoreState::IDLE)
            available++;
    }
    return available;
}

Core::Core() {
}

Core::Core(int id, unsigned int quantum, unsigned int delay, std::mutex* readyMutex, std::mutex* runningMutex, std::mutex* finishedMutex, std::vector<std::shared_ptr<screen::Screen>>* ready, std::vector<std::shared_ptr<screen::Screen>>* running, std::vector<std::shared_ptr<screen::Screen>>* finished) : readyMutex(readyMutex), runningMutex(runningMutex), finishedMutex(finishedMutex) {
    this->id = id;
    this->state = CoreState::IDLE;
    this->currScreen = nullptr;
    this->ready = ready;
    this->running = running;
    this->finished = finished;
    this->quantum = quantum;
    this->remainingQuantum = quantum;
    this->currCycle = 0;
    this->delay = delay + 1;
    this->cycleFinished = true;
}

void Core::work() {
    while (true) {
        // if delay cycles have passed
        if (currCycle % delay == 0) {
            if (this->state == CoreState::BUSY && currScreen != nullptr) {
                currScreen->setState(ProcessState::RUNNING);
                // preempt if rr
                if (remainingQuantum == 0 && quantum != 0) {
                    currScreen->setCore(-1);
                    {
                        std::lock_guard<std::mutex> runningLock(*runningMutex);
                        removeRunning(currScreen);
                    }
                    currScreen->setState(ProcessState::READY);
                    {
                        std::lock_guard<std::mutex> readyLock(*readyMutex);
                        addReady(currScreen);
                    }
                    currScreen = nullptr;
                    this->state = CoreState::IDLE;
                    remainingQuantum = quantum;
                    continue;
                }
                // std::cout << "CPU " << id << " doing work on " << currScreen->getName() << "\n";
                currScreen->print();
                if (quantum != 0)
                    remainingQuantum--;

                // process finished
                if (currScreen->isFinished()) {
                    {
                        std::lock_guard<std::mutex> runningLock(*runningMutex);
                        removeRunning(currScreen);
                    }
                    {
                        std::lock_guard<std::mutex> finishedLock(*finishedMutex);
                        addFinished(currScreen);
                    }
                    currScreen->setCore(-1);
                    currScreen->setEndTime();
                    currScreen->setState(ProcessState::TERMINATED);
                    // std::cout << "CPU " << id << " finished work on " << currScreen->getName() << "\n";
                    currScreen = nullptr;
                    this->state = CoreState::IDLE;
                    continue;
                }
            }
        }
        else
            currScreen->setState(ProcessState::WAITING);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        cycleFinished = true;
        while (cycleFinished) {
            // wait here for cycle update
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        currCycle += 1;
        // break;
    }
}

CoreState Core::getState() const {
    return this->state;
}

void Core::setState(const CoreState state) {
    this->state = state;
}

void Core::setScreen(const std::shared_ptr<screen::Screen> &s) {
    this->currScreen = s;
    this->state = CoreState::BUSY;
    currScreen->setCore(id);
    currScreen->setStartTime();
}

void Core::addFinished(const std::shared_ptr<screen::Screen>& s) {
    finished->push_back(s);
}

void Core::addReady(const std::shared_ptr<screen::Screen>& s) {
    ready->push_back(s);
}

void Core::removeRunning(const std::shared_ptr<screen::Screen>& s) {
    for (int i = 0; i < running->size(); i++) {
        if (running->at(i) == s) {
            running->erase(running->begin() + i);
        }
    }
}

bool Core::isCycleFinished() const {
    return cycleFinished;
}

void Core::setCycleFinished(bool cycleFinished) {
    this->cycleFinished = cycleFinished;
}

