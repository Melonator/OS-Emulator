#include "../include/cpu.h"
#include <iostream>
#include <thread>
using namespace cpu;

void coreThread(const std::shared_ptr<Core> &core) {
    core->work();
}

CPU::CPU() {

}

CPU::CPU(int numCores, unsigned int quantum, unsigned int delay, const std::string &algorithm, std::vector<std::shared_ptr<screen::Screen>>* ready, std::vector<std::shared_ptr<screen::Screen>>* running, std::vector<std::shared_ptr<screen::Screen>>* finished) {
    //Logic for initializing threads and other stuff
    // std::cout << "I have " << numCores << " core(s)!\n";
    this->numCores = numCores;
    this->algorithm = algorithm;
    this->quantum = quantum;
    for (int i = 0; i < numCores; i++) {
        std::shared_ptr<Core> core = std::make_shared<Core>(i, quantum, delay, ready, running, finished);
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


Core::Core() {
    this->id = -1;
    this->state = CoreState::IDLE;
    this->currScreen = nullptr;

}

Core::Core(int id, unsigned int quantum, unsigned int delay, std::vector<std::shared_ptr<screen::Screen>>* ready, std::vector<std::shared_ptr<screen::Screen>>* running, std::vector<std::shared_ptr<screen::Screen>>* finished) {
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
}

void Core::work() {
    while (true) {
        // if delay cycles have passed
        if (currCycle % delay == 0) {
            if (this->state == CoreState::BUSY && currScreen != nullptr) {
                // preempt if rr
                if (remainingQuantum == 0 && quantum != 0) {
                    currScreen->setCore(-1);
                    removeRunning(currScreen);
                    currScreen->setState(ProcessState::READY);
                    ready->push_back(currScreen);
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
                    currScreen->setCore(-1);
                    removeRunning(currScreen);
                    addFinished(currScreen);
                    currScreen->setEndTime();
                    currScreen->setState(ProcessState::TERMINATED);
                    // std::cout << "CPU " << id << " finished work on " << currScreen->getName() << "\n";
                    currScreen = nullptr;
                    this->state = CoreState::IDLE;
                    continue;
                }
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
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

void Core::setScreen(std::shared_ptr<screen::Screen> s) {
    this->currScreen = s;
    this->state = CoreState::BUSY;
    currScreen->setCore(id);
    currScreen->setStartTime();
}

void Core::addFinished(std::shared_ptr<screen::Screen> s) {
    finished->push_back(s);
}

void Core::removeRunning(std::shared_ptr<screen::Screen> s) {
    for (int i = 0; i < running->size(); i++) {
        if (running->at(i) == s) {
            running->erase(running->begin() + i);
        }
    }
}


