#include <filesystem>
#include <bitset>
#include <fstream>
#include <iostream>

typedef uint8_t Byte;
typedef uint16_t Word;

/*
0x0 - 0x02 = "NES"
0x3        = 0x1A - File format
0x4        = Number of 16 KB PRG-ROM banks
0x5        = Number of 8 KB CHR-ROM / VROM banks
0x6        = ROM Control Byte 1
                • Bit 0 - Indicates the type of mirroring used by the game
                where 0 indicates horizontal mirroring, 1 indicates
                vertical mirroring.
                • Bit 1 - Indicates the presence of battery-backed RAM at
                memory locations $6000-$7FFF.
                • Bit 2 - Indicates the presence of a 512-byte trainer at
                memory locations $7000-$71FF.
                • Bit 3 - If this bit is set it overrides bit 0 to indicate fourscreen mirroring should be used.
                • Bits 4-7 - Four lower bits of the mapper number.
0x7        = ROM Control Byte 2:
                • Bits 0-3 - Reserved for future usage and should all be 0.
                • Bits 4-7 - Four upper bits of the mapper number.
0x8        = Number of 8 KB RAM banks. For compatibility with previous. For compatibility with previous
                versions of the iNES format, assume 1 page of RAM when
                this is 0.
0x9 - 0xF = Reserved for future usage and should all be 0

512 byte trainer, if present, precedes the ROM data and is used to modify the game program. The
trainer is stored in the file in little-endian format. The trainer may be stored in the file in either
compressed or uncompressed form. If the trainer is stored in compressed form, it must be
decompressed before it is used. The decompressed data must fit entirely in the 512 bytes of the
trainer. If the trainer is stored in uncompressed form, it is used as is. If the trainer is not present,
this field is not present in the file.

The ROM data immediately follows the optional trainer, if present, or immediately follows the
header, if no trainer is present. The ROM data is stored in little-endian format. If the ROM data is
stored in compressed form, it must be decompressed before it is used. The decompressed data
must fit entirely in the PRG-ROM area of the cartridge. If the ROM data is stored in uncompressed
form, it is used as is.
*/

class ROM {
public:

    #pragma pack(push, 1) // Disable padding bytes
    struct Header {
        uint32_t magicNumbers;
        uint8_t numPRGROMBanks;
        uint8_t numCHRROMBanks;
        uint8_t controlStatus1;
        uint8_t controlStatus2;
        uint8_t numRAM;
        uint8_t unused[7];
    };
    #pragma pack(pop) // Re-enable padding

    bool LoadROMFile(const std::filesystem::path& path) {
        std::ifstream file(path, std::ios::binary | std::ios::ate); // Open file for reading in binary mode, and position the file pointer at the end

        if (!file) {
            std::cerr << "Unable to open file: " << path << std::endl;
            return false;
        }

        // Get file size
        std::streamsize size = file.tellg();
        file.seekg(0, std::ios::beg); // Move the file pointer to the beginning of the file

        // Read Magic numbers
        if (!file.read(reinterpret_cast<char*>(&hdr), sizeof(Header))) {
            return false;
        }

        // Read Trainer
        if (hdr.controlStatus1 & 0b00000100) {
            std::cout << "Reading trainor" << std::endl;
            if (!file.read(reinterpret_cast<char*>(Trainer), 512)) {
                return false;
            }
        }

        if (!file.read(reinterpret_cast<char*>(PRGROM), (hdr.numPRGROMBanks * 16 * 1024))) {
            return false;
        }

        if (!file.read(reinterpret_cast<char*>(CHRROM), (hdr.numCHRROMBanks * 8 * 1024))) {
            return false;
        }

        std::cout << "File loaded successfully. Read " << size << std::endl;

        DumpMemoryToFile(PRGROM, "PRGROM.dmp", 0x0000, 32*1024);
        DumpMemoryToFile(CHRROM, "CHROM.dmp", 0x0000, 8*1024);
        return true;
    }

    static void DumpMemoryToFile(Byte * mem, const std::filesystem::path& path, Word startAddress, Word endAddress) {
        std::ofstream outFile(path, std::ios::out); // Open the file for writing

        if (!outFile) {
            std::cerr << "Unable to open file: " << path << std::endl;
            return;
        }

        for (size_t i = startAddress; i < endAddress+1; i += 16) {
            // Write the memory address
            outFile << "0x" << std::hex << std::setw(4) << std::setfill('0') << i << ": ";

            // Write 16 bytes of memory in hexadecimal
            for (int j = 0; j < 16; j++) {
                outFile << std::hex << std::setw(2) << std::setfill('0') << (unsigned int)(unsigned char)mem[i + j] << " ";
            }

            outFile << std::endl;
        }

        outFile.close();
        std::cout << "Memory dumped to file: " << path << std::endl;
    }

private:
    Header hdr;
    Byte Trainer[512];
    Byte PRGROM[32*1024]; // 32K of ROM
    Byte CHRROM[8*1024]; // 8K of ROM
};