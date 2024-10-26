#include "../include/scheduler.h"

#include <cstring>
#include <format>
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
    this->cpu = cpu::CPU(numCores, quantum, delay, &readyMutex, &runningMutex, &finishedMutex, algorithm, this->ready, this->running, this->finished);
    this->generateProcess = false;
}

void Scheduler::run() {
    bool started = false;
    while (true) {

        // check for available cores
        for (int i = 0; i < cpu.getNumCores() && this->ready->size() > 0; i++) {
            // run processes
            std::lock_guard<std::mutex> readyLock(readyMutex);
            if (this->ready->size() > 0) {
                std::shared_ptr<cpu::Core> core = this->cpu.getCores().at(i);
                if (core->getState() == CoreState::IDLE) {
                    try {
                        std::shared_ptr<screen::Screen> process;
                        process = ready->front();
                        ready->erase(ready->begin());
                        if (process != nullptr) {
                            started = true;
                            core->setScreen(process);
                            std::lock_guard<std::mutex> runningLock(runningMutex);
                            addRunning(process);
                        }
                    } catch (const std::exception& e) {
                        std::cout << e.what() << std::endl;
                    }
                }
                // std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        }
        while (!this->cpu.allCyclesFinished()) {
            // wait
            // std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        if (started) {
            currCycle += 1;
            this->cpu.setAllCyclesFinished(false);
        }

        // scheduler-test process generation
        if (currCycle % processFreq == 0 && generateProcess) {
            unsigned int ins = minIns + (rand() % (maxIns - minIns + 1));
            std::shared_ptr<screen::Screen> p = std::make_shared<screen::Screen>("screen_" + std::to_string(processIndex), ins);
            processes->push_back(p);
            {
                std::lock_guard<std::mutex> readyLock(readyMutex);
                addProcess(p);
            }
            processIndex++;
            // std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        // std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

void Scheduler::startTest() {
    this->generateProcess = true;
}

void Scheduler::endTest() {
    this->generateProcess = false;
    // std::cout << "Generated processes: " << processes->size() << "\n";
}


void Scheduler::addProcess(const std::shared_ptr<screen::Screen> &process) {
    // Logic for adding process to queue
    // std::lock_guard<std::mutex> runningLock(runningMutex);
    this->ready->push_back(process);
}

void Scheduler::addRunning(std::shared_ptr<screen::Screen> s) {
    this->running->push_back(s);
}

std::string Scheduler::screenList() {
    // convert whole function to return string
    std::string list = "";

    int available = cpu.getAvailableCores();
    int coresUsed = cpu.getNumCores() - available;
    list += std::format("CPU Utilization: {0:.2f}%\n", cpu.getUtilization() * 100);
    // printf("CPU Utilization: %.2f%%\n", cpu.getUtilization() * 100);
    list += "Cores used: " + std::to_string(coresUsed) + "\n";
    // printf("Cores used: %d\n", coresUsed);
    list += "Cores available: " + std::to_string(available) + "\n";
    // printf("Cores available: %d\n", available);

    // for (int i = 0; i < processes->size(); i++) {
    //     std::shared_ptr<screen::Screen> p = processes->at(i);
    //     if (p != nullptr) {
    //         list += p->toString() + "\n";
    //     }
    // }
    list += "\n--------------------------\n";
    // std::cout << "\n--------------------------\n";
    list += "Running processes:\n";
    // std::cout << "Running processes:\n";
    try {
            std::lock_guard<std::mutex> runningLock(runningMutex);
            for (int i = 0; i < running->size(); i++) {
                std::shared_ptr<screen::Screen> p = running->at(i);
                if (p != nullptr) {
                    list += running->at(i)->toString() + "\n";
                    // std::cout << running->at(i)->toString() << "\n";
                }
            }
        // for (auto & i : *this->running) {
        //     std::cout << i->toString() << "\n";
        // }
    } catch (const std::exception& e) {
        std::cout << e.what() << std::endl;
    }

    list += "\nFinished processes:\n";
    // std::cout << "\nFinished processes:\n";
    try {
        std::lock_guard<std::mutex> finishedLock(finishedMutex);
        for (int i = 0; i < finished->size(); i++) {
            std::shared_ptr<screen::Screen> p = finished->at(i);
            if (p != nullptr) {
                list += finished->at(i)->toString() + "\n";
            }
            // std::cout << finished->at(i)->toString() << "\n";
        }
        // for (auto & i : *this->finished) {
        //     std::cout << i->toString() << "\n";
        // }
    } catch (const std::exception& e) {
        std::cout << e.what() << std::endl;
    }
    list += "--------------------------\n";
    // std::cout << "--------------------------\n";

    return list;
}


void Scheduler::printList() {
    std::cout << screenList();
}

void Scheduler::saveList() {
    // save to file csopesy-log.txt
    std::ofstream file;
    file.open("csopesy-log.txt");
    file << screenList();
    file.close();
}


unsigned int Scheduler::getMinIns() const {
    return this->minIns;
}

unsigned int Scheduler::getMaxIns() const {
    return this->maxIns;
}

