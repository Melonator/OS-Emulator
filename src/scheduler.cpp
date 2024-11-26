#include "../include/scheduler.h"

#include <cmath>
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
    std::string configs[10];
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
    this->quantum = std::stol(configs[2]);
    this->processes = processes;
    this->processFreq = std::stoi(configs[3]) + 1;
    this->minIns = std::stol(configs[4]);
    this->maxIns = std::stol(configs[5]);
    this->delay = std::stol(configs[6]);
    this->memory = std::stol(configs[7]);
    this->blockSize = std::stol(configs[8]);

    size_t minMem = std::stol(configs[9]);
    size_t maxMem = std::stol(configs[10]);
    minMem = log2(minMem);
    maxMem = log2(maxMem);

    this->minMemPerProc = minMem;
    this->maxMemPerProc = maxMem;

    // std::cout << numCores << " " << algorithm << " " << quantum << " " << processFreq << " " << minIns << " " << maxIns << " " << delay << std::endl;
    if (algorithm == "fcfs")
        quantum = 0;
    this->currCycle = 0;
    this->processIndex = 0;
    this->ready = new std::vector<std::shared_ptr<screen::Screen>>();
    this->running = new std::vector<std::shared_ptr<screen::Screen>>();
    this->finished = new std::vector<std::shared_ptr<screen::Screen>>();
    this->flatModel = std::make_shared<allocator::FlatModel>(memory, blockSize);
    this->pagingModel = std::make_shared<allocator::Paging>(memory, blockSize);
    this->cpu = cpu::CPU(numCores, quantum, delay, &readyMutex, &runningMutex, &finishedMutex, algorithm, this->ready, this->running, this->finished);
    this->generateProcess = false;
    this->screenLS = false;
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
                            // add allocator logic here
                            std::string pName = process->getName();
                            if (pagingModel->isAllocated(pName)) {
                                started = true;
                                core->setScreen(process);
                                std::lock_guard<std::mutex> runningLock(runningMutex);
                                addRunning(process);
                            }
                            else if (pagingModel->inBackingStore(pName)) { // check if in backing store
                                if (pagingModel->canAllocate(process->getMemoryRequired())) {
                                    pagingModel->getFromBackingStore(pName, currCycle);
                                    started = true;
                                    core->setScreen(process);
                                    std::lock_guard<std::mutex> runningLock(runningMutex);
                                    addRunning(process);
                                }
                                else {
                                    // move oldest to backing
                                    std::string oldest = pagingModel->getOldestProcess();
                                    std::lock_guard<std::mutex> runningLock(runningMutex);
                                    if (isRunning(oldest))
                                        preempt(oldest);
                                    pagingModel->moveToBackingStore(oldest);
                                    pagingModel->getFromBackingStore(pName, currCycle);
                                    started = true;
                                    core->setScreen(process);
                                    addRunning(process);
                                }
                            }
                            else { // not in backing store and new process
                                // do oldest process move to backing
                                std::string oldest = pagingModel->getOldestProcess();
                                std::lock_guard<std::mutex> runningLock(runningMutex);
                                if (isRunning(oldest))
                                    preempt(oldest);
                                pagingModel->moveToBackingStore(oldest);
                                started = true;
                                core->setScreen(process);
                                addRunning(process);
                            }
                            // if (process->getMemLoc() == nullptr)
                            //     process->setMemLoc(flatModel->allocate(process->getMemoryRequired(), process->getName()));
                            // if (process->getMemLoc() != nullptr) {
                            //     started = true;
                            //     core->setScreen(process);
                            //     std::lock_guard<std::mutex> runningLock(runningMutex);
                            //     addRunning(process);
                            // }
                            // else {
                            //     this->ready->push_back(process);
                            // }
                        }
                    } catch (const std::exception& e) {
                        std::cout << e.what() << std::endl;
                    }
                }
                // std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        }
        if (started) {
            cpu.work();
        }
        while (!this->cpu.allCyclesFinished()) {
            // wait
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }

        if (started) {
            {
                // deallocate memory when finished
                std::lock_guard<std::mutex> finishedLock(finishedMutex);
                for (int i = 0; i < finished->size(); i++) {
                    std::shared_ptr<screen::Screen> p = finished->at(i);
                    if (p != nullptr) {
                        flatModel->deallocate(p->getMemLoc());
                        p->setMemLoc(nullptr);
                    }
                }
            }


            // if (currCycle % quantum == 0) {
            //     // visualize memory per qq
            //     std::ofstream logFile;
            //     std::string fileName = "memory_stamp_" + std::to_string(currCycle) + ".txt";
            //     logFile.open(fileName, std::ios_base::app);
            //     logFile << flatModel->visualizeMemory();
            //     logFile.close();
            // }

            if (screenLS) {
                printList();
                screenLS = false;
            }
            currCycle += 1;
            this->cpu.setAllCyclesFinished(false);
        }

        // scheduler-test process generation
        if (currCycle % processFreq == 0 && generateProcess) {
            std::shared_ptr<screen::Screen> p = createProcess("screen_" + std::to_string(processIndex));
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

std::shared_ptr<screen::Screen> Scheduler::createProcess(std::string name) {
    unsigned int ins = getMinIns() + (rand() % (getMaxIns() - getMinIns() + 1));
    size_t memPow = minMemPerProc + (rand() % (maxMemPerProc - minMemPerProc + 1));
    size_t memPerProc = pow(2, memPow);
    std::shared_ptr<screen::Screen> p = std::make_shared<screen::Screen>(name, ins, memPerProc);
    return p;
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

void Scheduler::setScreenLS() {
    this->screenLS = true;
}

std::vector<std::string> Scheduler::getRunningNames() {
    std::lock_guard<std::mutex> runningLock(runningMutex);
    std::vector<std::string> names;
    for (int i = 0; i < running->size(); i++) {
        names.push_back(running->at(i)->getName());
    }
    return names;
}

bool Scheduler::isRunning(std::string name) {
    for (size_t i = 0; i < running->size(); i++) {
        if (running->at(i)->getName() == name) {
            return true;
        }
    }
    return false;
}

void Scheduler::preempt(std::string name) {
    for (size_t i = 0; i < cpu.getCores().size(); i++) {
        std::shared_ptr<cpu::Core> core = cpu.getCores().at(i);
        if (core->getState() == CoreState::BUSY) {
            std::shared_ptr<screen::Screen> p = core->getCurrScreen();
            if (p->getName() == name) {
                core->preempt();
                break;
            }
        }
    }
}


