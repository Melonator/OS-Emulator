#include "../include/cpu.h"
#include <iostream>
#include <thread>
using namespace cpu;

void coreThread(const std::shared_ptr<Core> &core) {
    core->work();
}

CPU::CPU() {

}

CPU::CPU(int numCores, std::vector<std::shared_ptr<screen::Screen>>* running, std::vector<std::shared_ptr<screen::Screen>>* finished) {
    //Logic for initializing threads and other stuff
    // std::cout << "I have " << numCores << " core(s)!\n";
    this->numCores = numCores;
    for (int i = 0; i < numCores; i++) {
        std::shared_ptr<Core> core = std::make_shared<Core>(i, running, finished);
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

Core::Core(int id, std::vector<std::shared_ptr<screen::Screen>>* running, std::vector<std::shared_ptr<screen::Screen>>* finished) {
    this->id = id;
    this->state = CoreState::IDLE;
    this->currScreen = nullptr;
    this->running = running;
    this->finished = finished;
}

void Core::work() {
    while (true) {
        if (this->state == CoreState::IDLE && currScreen != nullptr) {
            // do stuff
            // std::cout << "CPU " << id << " doing work on " << currScreen->getName() << "\n";
            this->state = CoreState::BUSY;
            currScreen->setCore(id);
            currScreen->setStartTime();
            currScreen->print();
            currScreen->setEndTime();
            // std::this_thread::sleep_for(std::chrono::milliseconds(2000));
            // std::cout << "CPU " << id << " finished work on " << currScreen->getName() << "\n";
            currScreen->setCore(-1);
            removeRunning(currScreen);
            addFinished(currScreen);
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


