#include "../include/scheduler.h"

#include <iostream>
#include <thread>
using namespace scheduler;

Scheduler::Scheduler() {
    this->ready = new std::vector<std::shared_ptr<screen::Screen>>();
    this->running = new std::vector<std::shared_ptr<screen::Screen>>();
    this->finished = new std::vector<std::shared_ptr<screen::Screen>>();
    this->cpu = cpu::CPU(4, this->running, this->finished);
}

void Scheduler::run() {
    while (true) {
        // Logic for running processes
        for (int i = 0; i < cpu.getNumCores(); i++) {
        // if (this->ready->size() > 0) {
            // run processes
            // for (int i = 0; i < cpu.getNumCores(); i++) {
            if (this->ready->size() > 0) {
                std::shared_ptr<cpu::Core> core = this->cpu.getCores().at(i);
                if (core->getState() == CoreState::IDLE) {
                    std::shared_ptr<screen::Screen> process;
                    try {
                        process = ready->front();
                        ready->erase(ready->begin());
                        core->setScreen(process);
                        addRunning(process);
                    } catch (const std::exception& e) {
                        std::cout << e.what() << std::endl;
                    }
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            }
        }
    }
}

void Scheduler::addProcess(const std::shared_ptr<screen::Screen> &process) {
    // Logic for adding process to queue
    this->ready->push_back(process);
}

void Scheduler::addRunning(std::shared_ptr<screen::Screen> s) {
    this->running->push_back(s);
}

void Scheduler::printList() {
    std::cout << "--------------------------\n";
    std::cout << "Running processes:\n";
    try {
        for (int i = 0; i < this->running->size(); i++) {
            std::cout << this->running->at(i)->toString() << "\n";
        }
    } catch (const std::exception& e) {
        std::cout << e.what() << std::endl;
    }

    std::cout << "\nFinished processes:\n";
    try {
        for (int i = 0; i < this->finished->size(); i++) {
            std::cout << this->finished->at(i)->toString() << "\n";
        }
    } catch (const std::exception& e) {
        std::cout << e.what() << std::endl;
    }
    std::cout << "--------------------------\n";
}
