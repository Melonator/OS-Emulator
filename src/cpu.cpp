#include "../include/cpu.h"
#include <iostream>
#include <thread>
using namespace cpu;

void coreThread(const std::shared_ptr<Core> &core) {
    core->work();
}

CPU::CPU(int numCores) {
    //Logic for initializing threads and other stuff
    // std::cout << "I have " << numCores << " core(s)!\n";
    this->numCores = numCores;
    this->running = std::vector<std::shared_ptr<screen::Screen>>();
    this->finished = std::vector<std::shared_ptr<screen::Screen>>();
    for (int i = 0; i < numCores; i++) {
        std::shared_ptr<Core> core = std::make_shared<Core>(i);
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

void CPU::addRunning(std::shared_ptr<screen::Screen> s) {
    this->running.push_back(s);
}


Core::Core() {
    this->id = -1;
    this->state = CoreState::IDLE;
    this->currScreen = nullptr;
}

Core::Core(int id) {
    this->id = id;
    this->state = CoreState::IDLE;
    this->currScreen = nullptr;
}

void Core::work() {
    while (true) {
        if (this->state == CoreState::IDLE && currScreen != nullptr) {
            // do stuff
            // std::cout << "CPU " << id << " doing work on " << currScreen->getName() << "\n";
            this->state = CoreState::BUSY;
            currScreen->setCore(id);
            currScreen->print();
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            // std::cout << "CPU " << id << " finished work on " << currScreen->getName() << "\n";
            currScreen->setCore(-1);
            currScreen = nullptr;
            this->state = CoreState::IDLE;
        }
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
}
