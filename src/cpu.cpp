#include "../include/cpu.h"
#include <iostream>

using namespace cpu;

CPU::CPU(int numCores) {
    //Logic for initializing threads and other stuff
    std::cout << "I have " << numCores << " core(s)!\n";
}

void CPU::test() {
    std::cout << "test";
}




