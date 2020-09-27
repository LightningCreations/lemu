#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

class NESCart {
private:
    uint8_t prgsize; // Number of 16 KiB blocks
    uint8_t chrsize; // Number of 8 KiB blocks
    uint32_t mapper; // Mapper number
    uint8_t *prg;
    uint8_t *chr;
public:
    NESCart(uint8_t, uint8_t, uint32_t, uint8_t*, uint8_t*);
    uint8_t get(uint16_t addr);
    void set(uint16_t addr, uint8_t data);
};

NESCart::NESCart(uint8_t prgsize, uint8_t chrsize, uint32_t mapper, uint8_t *prg, uint8_t *chr) {
    this->prgsize = prgsize;
    this->chrsize = chrsize;
    this->mapper = mapper;
    this->prg = prg;
    this->chr = chr;
}

uint8_t ppuController;
uint8_t ppuStatus;

uint8_t NESCart::get(uint16_t addr) {
    if(addr == 0x2002) {
        return ppuStatus;
    }
    switch(mapper) {
    case 0:
        if(addr >= 0xC000) {
            if(prgsize == 2) return prg[addr - 0x8000];
            else return prg[addr - 0xC000];
        } else if(addr >= 0x8000) {
            return prg[addr - 0x8000];
        } else {
            fprintf(stderr, "Unrecognized PRG address $%04X in mapper 0 (read)!\n", addr);
            exit(1);
        }
    default:
        fprintf(stderr, "Unrecognized mapper id %d (read)!\n", mapper);
        exit(1);
    }
}

void NESCart::set(uint16_t addr, uint8_t data) {
    if(addr == 0x2000) {
        ppuController = data;
        return;
    }
    switch(mapper) {
    case 0:
        if(addr >= 0x8000) {
            return; // Don't do anything meaningful
        } else {
            fprintf(stderr, "Unrecognized PRG address $%04X in mapper 0 (write)!\n", addr);
            exit(1);
        }
    default:
        fprintf(stderr, "Unrecognized mapper id %d (write)!\n", mapper);
        exit(1);
    }
}

static NESCart *cart;

static bool active = false;
static bool fullCycle = false;

static uint8_t regA;
static uint8_t regX;
static uint8_t regP;
static uint16_t regPC;
static uint8_t regSP;

void NESEmuLoadROM(std::string romname) {
    FILE *romfile = fopen(romname.c_str(), "rb");
    fseek(romfile, 0, SEEK_END);
    size_t romsize = ftell(romfile);
    fseek(romfile, 0, SEEK_SET);
    uint8_t *romdata = (uint8_t*) malloc(romsize);
    fread((void*) romdata, 1, romsize, romfile);
    fclose(romfile);

    if(romdata[0] != 'N' || romdata[1] != 'E' || romdata[2] != 'S' || romdata[3] != 0x1a) {
        fprintf(stderr, "Not a NES ROM!\n");
        return;
    }

    bool nes2 = (romdata[7] & 0x0c) == 8;
    if(nes2) fprintf(stderr, "Warning: NES 2.0 not fully supported!\n");

    uint8_t prgsize = romdata[4];
    uint8_t chrsize = romdata[5];
    uint8_t *prg = (uint8_t*) malloc(prgsize * 16384);
    uint8_t *chr = (uint8_t*) ((chrsize != 0) ? malloc(chrsize * 8192) : nullptr);
    memcpy(prg, &(romdata[0x10]), prgsize * 16384);
    if(chr) memcpy(chr, &(romdata[0x10+prgsize]), chrsize * 8192);

    cart = new NESCart(prgsize, chrsize, ((romdata[6] & 0xf0) >> 4) | (romdata[7] & 0xf0), prg, chr);
    regSP = 0xFD;
    regPC = cart->get(0xFFFC) | (cart->get(0xFFFD) << 8);

    free((void*) romdata);

    active = true;
}

void NESEmuDoFrame() {
    if(!active) return;

    int numCycles = 0;
    while(numCycles < (29780 + (fullCycle ? 1 : 0))) {
        uint8_t instruction = cart->get(regPC); regPC++; numCycles++;
        switch(instruction) {
        case 0x10: {
            int8_t offset = cart->get(regPC); regPC++; numCycles++;
            // And now, since we aren't the 6502, we have to hack to get cycle accuracy.
            if(regP & 0x80) break;
            cart->get(regPC); uint16_t newPC = regPC + offset; numCycles++; // Dummy read!
            if((newPC & 0xFF00) == (regPC & 0xFF00)) { regPC = newPC; break; } // Page is fine
            cart->get((regPC & 0xFF00) | (newPC & 0x00FF)); regPC = newPC; numCycles++; // Page wasn't fine, so weird dummy read!
            break;
        }
        case 0x78:
            regP |= 0x04; cart->get(regPC); numCycles++; // Set interrupt flag
            break;
        case 0x8D: {
            uint16_t address = cart->get(regPC); regPC++; numCycles++;
            address |= (cart->get(regPC)) << 8; regPC++; numCycles++;
            cart->set(address, regA); numCycles++;
            break;
        }
        case 0x9A:
            regSP = regX; cart->get(regPC); numCycles++;
            break;
        case 0xA2:
            regX = cart->get(regPC); regPC++; numCycles++;
            if(regX == 0) regP |= 0x02; else regP &= 0xFD;
            if(regX & 0x80) regP |= 0x80; else regP &= 0x7F;
            break;
        case 0xA9:
            regA = cart->get(regPC); regPC++; numCycles++;
            if(regA == 0) regP |= 0x02; else regP &= 0xFD;
            if(regA & 0x80) regP |= 0x80; else regP &= 0x7F;
            break;
        case 0xAD: {
            uint16_t address = cart->get(regPC); regPC++; numCycles++;
            address |= (cart->get(regPC)) << 8; regPC++; numCycles++;
            regA = cart->get(address); numCycles++;
            if(regA == 0) regP |= 0x02; else regP &= 0xFD;
            if(regA & 0x80) regP |= 0x80; else regP &= 0x7F;
            break;
        }
        case 0xD8:
            regP &= 0xF7; cart->get(regPC); numCycles++; // Clear unused decimal flag
            break;
        default:
            fprintf(stderr, "Unknown instruction 0x%02X!\n", instruction);
            exit(1); // Temporary
        }
    }
    fullCycle = !fullCycle;
}
