#include <iostream>
#include <filesystem>
#include <iomanip>
#include <fstream>
#include "ROM.h"

typedef uint8_t Byte;
typedef uint16_t Word;

class Memory {
public:
    Memory() {
        // Reset ISR: Default: $FCE2
        Mem[0xFFFC] = 0xE2;
        Mem[0xFFFD] = 0xFC;
    }
    // Memory map: https://sta.c64.org/cbm64mem.html
    // $0000-$00FF - Zero page
    // $0100-$01FF - Stack
    // $0200-$07FF - RAM

    // $0400-$07FF - Default Screen Memory

    // $8000-$BFFF - Cartridge ROM

    Byte FetchByte(Word address) {
        return Mem[address];
    }

private:
    Byte Mem[0xFFFF] = {0x0}; // 64K of memory
};

class CPU_6502 {
public:
    void Reset() {
        PC = 0xFFFC; // Reset vector
    }

    void Execute(int Cycles, Memory& mem) {
        while (Cycles > 0) {
            Byte opCode = mem.FetchByte(PC++);
            Cycles--; // Read opcode takes 1 cycle
            switch (opCode) {
            default: {
                std::cout << "Unknown opcode: " << std::hex << (int)opCode << std::endl;
                break;
            }
            }
        }
    }

private:
    Word PC; // Program Counter
    Byte SP; // Stack Pointer
    Byte SR; // Stack Register

    Byte AC; // Accumulator
    Byte X; // Index Register X
    Byte Y; // Index Register Y

    Byte N : 1; // Negative
    Byte V : 1; // Overflow
    Byte B : 1; // Break
    Byte D : 1; // Decimal
    Byte I : 1; // Interrupt Disable
    Byte Z : 1; // Zero
    Byte C : 1; // Carry
};


int main(int argc, char** argv) {
    std::cout << "Booting up 6502" << std::endl;
    CPU_6502 cpu;
    Memory mem;
    ROM rom;

    if (argc > 1) {
        rom.LoadROMFile(argv[1]);
        // mem.DumpMemoryToFile("ROM.dmp", 0x8000);
    }

    // cpu.Execute(2, mem);
    return 0;
}