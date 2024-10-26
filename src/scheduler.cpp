#include "../include/scheduler.h"

#include <cstring>
#include <fstream>
#include <iostream>
#include <thread>
using namespace scheduler;

Scheduler::Scheduler() {

}

Scheduler::Scheduler(std::vector<std::shared_ptr<screen::Screen>>* processes) {
    // open config file
    std::ifstream file;
    file.open("../src/config.txt");
    std::string line;
    std::string configs[7];
    int i = 0;
    while (getline(file, line)) {
        char *ptr = strtok(line.data(), " ");
        ptr = strtok(NULL, " ");
        configs[i] = ptr;
        i++;
    }
    file.close();

    int numCores = std::stoi(configs[0]);
    std::string algorithm = configs[1].substr(1, configs[1].size() - 2);
    unsigned int quantum = std::stol(configs[2]);
    this->processes = processes;
    this->processFreq = std::stoi(configs[3]) + 1;
    this->minIns = std::stol(configs[4]);
    this->maxIns = std::stol(configs[5]);
    this->delay = std::stol(configs[6]);
    // std::cout << numCores << " " << algorithm << " " << quantum << " " << processFreq << " " << minIns << " " << maxIns << " " << delay << std::endl;
    if (algorithm == "fcfs")
        quantum = 0;
    this->currCycle = 0;
    this->processIndex = 0;
    this->ready = new std::vector<std::shared_ptr<screen::Screen>>();
    this->running = new std::vector<std::shared_ptr<screen::Screen>>();
    this->finished = new std::vector<std::shared_ptr<screen::Screen>>();
    this->cpu = cpu::CPU(numCores, quantum, delay, algorithm, this->ready, this->running, this->finished);
    this->generateProcess = false;
}

void Scheduler::run() {
    while (true) {
        // scheduler-test process generation
        if (currCycle % processFreq == 0 && generateProcess) {
            unsigned int ins = minIns + (rand() % (maxIns - minIns + 1));
            std::shared_ptr<screen::Screen> p = std::make_shared<screen::Screen>("screen_" + std::to_string(processIndex), ins);
            processes->push_back(p);
            addProcess(p);
            processIndex++;
        }

        // check for available cores
        for (int i = 0; i < cpu.getNumCores(); i++) {
            // run processes
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
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        }
        while (!this->cpu.allCyclesFinished()) {
            // wait
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        currCycle += 1;
        this->cpu.setAllCyclesFinished(false);
    }
}

void Scheduler::startTest() {
    this->generateProcess = true;
}

void Scheduler::endTest() {
    this->generateProcess = false;
}


void Scheduler::addProcess(const std::shared_ptr<screen::Screen> &process) {
    // Logic for adding process to queue
    this->ready->push_back(process);
}

void Scheduler::addRunning(std::shared_ptr<screen::Screen> s) {
    this->running->push_back(s);
}

void Scheduler::printList() {
    int available = cpu.getAvailableCores();
    int coresUsed = cpu.getNumCores() - available;

    printf("CPU Utilization: %.2f%%\n", cpu.getUtilization() * 100);
    printf("Cores used: %d\n", coresUsed);
    printf("Cores available: %d\n", available);

    std::cout << "\n--------------------------\n";
    std::cout << "Running processes:\n";
    try {
        for (int i = 0; i < running->size(); i++) {
            std::cout << running->at(i)->toString() << "\n";
        }
        // for (auto & i : *this->running) {
        //     std::cout << i->toString() << "\n";
        // }
    } catch (const std::exception& e) {
        std::cout << e.what() << std::endl;
    }

    std::cout << "\nFinished processes:\n";
    try {
        for (int i = 0; i < finished->size(); i++) {
            std::cout << finished->at(i)->toString() << "\n";
        }
        // for (auto & i : *this->finished) {
        //     std::cout << i->toString() << "\n";
        // }
    } catch (const std::exception& e) {
        std::cout << e.what() << std::endl;
    }
    std::cout << "--------------------------\n";
}

unsigned int Scheduler::getMinIns() const {
    return this->minIns;
}

unsigned int Scheduler::getMaxIns() const {
    return this->maxIns;
}

