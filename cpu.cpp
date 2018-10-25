#include <iostream>
#include <stdexcept>
#include <fstream> 
#include <vector>
#include <iomanip>
#include "cpu.h"


using namespace std;

CPU::CPU() {
    // init cpu and its memory management
    
    
    this->pc = memManager.get_mem_start(); // where the first instruction is loaded
    this->I = 0;
    this->V.resize(REGISTERS);

    this -> current_opcode = 0x0000;
}

CPU::~CPU() {
}

void CPU::loadProgram(string pathToProgram) {
	
	ifstream ifs(pathToProgram, ios::binary | ios::ate);
    ifstream::pos_type pos = ifs.tellg();

    if(pos > memManager.get_real_mem_size()) {
    	throw runtime_error("Game too big for memory.");
    }

    vector<char> result(pos);

    ifs.seekg(0, ios::beg);
    ifs.read(&result[0], pos);

    // here we copy into memory
    // MMU will deal with the right place to write 
    // (basically starting from 0x200)
    for(int i=0; i<pos; i++) {
    	memManager.set_mem(i+0x200, result[i]);
    	//cout << memManager.get_mem_at(memManager.get_mem_start()+i) << endl;
    }
    
}

unsigned short CPU::fetch_opcode() {
    /*
        Since opcode is a short and so it has a dimension of 2 bytes.
        An opcode is 2byte long, so we need to get two byte from the 
        memory and merge them together. For instance:
            . memory[pc]     == 0xA2 -> 10100010
            . memory[pc + 1] == 0xF0 -> 11110000
            . 10100010 << 8 == 10100010-00000000
            . 10100010-00000000 | 11110000 ==  10100010-11110000 
              which is 0xA2F0: our opcode
    */
    unsigned short opcode = memManager.get_mem_at(pc) << 8 | memManager.get_mem_at(pc+1);
    this -> current_opcode = opcode;

    return opcode;
}


void CPU::execute_opcode(unsigned short opcode){
    // cout << "Exec 0x" << hex << (opcode) << endl;
    OpcodeFunction opcode_handler = this->opcode_table.at(opcode & 0xF000);
    
    if(opcode_handler == NULL) {
        cerr << "Not implemented, opcode 0x" << hex << opcode << endl;
        exit (EXIT_FAILURE);
    }

    opcode_handler(this, opcode);
    //this -> pc += 2; // we go to the next instruction
}

void CPU::set_reg(int reg, unsigned char value){
    //std::vector<unsigned char>::iterator nth = V.begin()+reg;
    this -> V[reg] = value;
}

devices::Screen& CPU::get_graphic(){
    return this->gfx;
}

void CPU::update_delay_timer(){
    if(this->delay_timer > 0) {
        this->delay_timer -= 1;
    }
}

void CPU::update_sound_timer(){
    if(this->sound_timer > 0){
        if(this->sound_timer == 1) {
            printf("BEEEEP");
            this->sound_timer -= 1;
        }
    }
}

devices::Keyboard& CPU::get_keyboard() {
    return this->keys;
}

void CPU::debug() {
    for(int i=0; i < this->REGISTERS; i++) {
        printf("V[%x]=%x | ", i, this->V[i]);
    }
    printf("\n");
    //printf("delay_timer=%x", this->delay_timer);
}

/*
    Opcode functions implementation starts here
*/

void CPU::_0_opcode(CPU *cpu, unsigned short opcode){
    switch(opcode & 0x000F){    
        case 0x0000: // 0x00E0: Clears the screen
            
            cpu -> gfx.clearScreen();
            cpu->pc += 2;
            break;
        case 0x000E: // 0x00EE: Returns from subroutine
            cpu -> pc = cpu -> memManager.pop();
            cpu->pc += 2;
            break;
        default:
            cerr << "Not implemented, opcode 0x" << hex << opcode << endl;
            exit (EXIT_FAILURE);
    }
}

void CPU::_1_opcode(CPU *cpu, unsigned short opcode){
    // opcode: 1NNN goto NNN; Jumps to address NNN.
    // We simply need to setup the pc to the jump address
    cpu -> pc = opcode & 0x0FFF;
}

void CPU::_2_opcode(CPU *cpu, unsigned short opcode){
    // opcode: 2NNN *(0xNNN)()  Calls subroutine at NNN.
    
    cpu -> memManager.push(cpu->pc);
    cpu -> pc = opcode & 0x0FFF;
}

void CPU::_3_opcode(CPU *cpu, unsigned short opcode) {
    // 3XNN Cond if(Vx==NN) Skips the next instruction if VX equals NN.
    // (Usually the next instruction is a jump to skip a code block)
    unsigned short x = (opcode & 0x0F00) >> 8;
    //printf("V[%x] = 0x%x == 0x%x\n",x, cpu->V[x], opcode & 0x0FF);
    if(cpu->V[x] == (opcode & 0x0FF)) {
        cpu->pc += 4;
    } else {
        cpu->pc += 2;
    }
}
void CPU::_4_opcode(CPU *cpu, unsigned short opcode){
    // 4XNN Cond if(Vx!=NN)  Skips the next instruction if VX doesn't equal NN. 
    // (Usually the next instruction is a jump to skip a code block)
    unsigned short x = (opcode & 0x0F00) >> 8;
    if(cpu->V[x] != (opcode & 0x0FF)) {
        cpu->pc += 4;
    } else {
        cpu->pc += 2;
    }
}

void CPU::_5_opcode(CPU *cpu, unsigned short opcode){
    // 5XY0 Cond if(Vx==Vy)  Skips the next instruction if VX equals VY. 
    // (Usually the next instruction is a jump to skip a code block)
    unsigned short x = (opcode & 0x0F00) >> 8;
    unsigned short y = (opcode & 0x00F0) >> 4;
    if(cpu->V[x] == cpu->V[y]) {
        cpu->pc += 4;
    } else {
        cpu->pc += 2;
    }
}

void CPU::_6_opcode(CPU *cpu, unsigned short opcode){
    // 6XNN Const   Vx = NN Sets VX to NN.
    unsigned short x = (opcode & 0x0F00) >> 8;
    cpu->set_reg(x, (opcode & 0X00FF));
    cpu->pc += 2;
    
}

void CPU::_7_opcode(CPU *cpu, unsigned short opcode){
    // 7XNN Const Vx += NN    Adds NN to VX. (Carry flag is not changed)
    unsigned short x = (opcode & 0x0F00) >> 8;
    //printf("V[%x] = %x\n", x, cpu->V[x]);
    cpu->set_reg(x, cpu->V[x]+(opcode & 0X00FF));
    //printf("V[%x] = %x\n", x, cpu->V[x]);
    cpu->pc += 2;
}

void CPU::_8_opcode(CPU *cpu, unsigned short opcode){
    unsigned short x = (opcode & 0x0F00) >> 8;
    unsigned short y = (opcode & 0x00F0) >> 4;
    unsigned short f = 16; // to access Vf
    
    switch(opcode & 0X000F) {
        
        
        case 0x0000:
            // 8XY0 Assign Vx=Vy Sets VX to the value of VY.
            cpu->set_reg(x, cpu->V[y]);
            cpu->pc += 2;
        break;
        case 0x0001:
            // 8XY1 BitOp Vx=Vx|Vy Sets VX to VX or VY. 
            //(Bitwise OR operation)
            cpu->set_reg(x, cpu->V[x]|cpu->V[y]);
            cpu->pc += 2;
        break;
        case 0x0002:
            //  BitOp  Vx=Vx&Vy Sets VX to VX and VY. 
            // (Bitwise AND operation)
            cpu->set_reg(x, cpu->V[x]&cpu->V[y]);
            cpu->pc += 2;
        break;
        case 0x0003:
            // 8XY3 BitOp Vx=Vx^Vy Sets VX to VX xor VY.
            cpu->set_reg(x, cpu->V[x]^cpu->V[y]);
            cpu->pc += 2;
        break;
        case 0x0004:
            //8XY4 Math Vx += Vy    
            //Adds VY to VX. VF is set to 1 when there's a carry, 
            // and to 0 when there isn't.

            // Remember that register are 1byte big...
            if(cpu->V[y] > 0xFF - cpu->V[x]) {
                cpu->set_reg(16, 1);
            } else {
                cpu->set_reg(16, 0);
            }

            cpu->set_reg(x, cpu->V[x]+cpu->V[y]);
            cpu->pc += 2;
        break;
        case 0x0005:
            // 8XY5 Math  Vx -= Vy VY is subtracted from VX. 
            // VF is set to 0 when there's a borrow, and 1 when there isn't.
            if(cpu->V[y] > cpu->V[x]) {
                cpu->set_reg(f, 0);
            } else {
                cpu->set_reg(f, 1);
            }
            cpu->set_reg(x, cpu->V[x]-cpu->V[y]);
            cpu->pc += 2;
        break;

        case 0x0006:
            // 8xy6 - SHR Vx {, Vy}
            // Set Vx = Vx SHR 1.
            cpu->set_reg(x, cpu->V[x]>>1);
            cpu->pc += 2;
            cpu->pc+=2;
        break;
        case 0x0007:
            // 8xy7 - SUBN Vx, Vy
            // Set Vx = Vy - Vx, set VF = NOT borrow.
            // If Vy > Vx, then VF is set to 1, otherwise 0. 
            // Then Vx is subtracted from Vy, and the results stored in Vx.
            if(cpu->V[y] > cpu->V[x]) {
                cpu->set_reg(f, 1);
            } else {
                cpu->set_reg(f, 0);
            }
            cpu->set_reg(x, cpu->V[y]-cpu->V[x]);
            cpu->pc += 2;
        break;
        case 0x000E:
            // 8xyE - SHL Vx {, Vy}
            // Set Vx = Vx SHL 1.
            // If the most-significant bit of Vx is 1, then VF is set to 1, 
            // otherwise to 0. Then Vx is multiplied by 2.
            if(cpu->V[x] & 0x1) {
                cpu->set_reg(f,1);
            } else {
                cpu->set_reg(f,0);
            }
            cpu->set_reg(x, cpu->V[x] << 1);
            cpu->pc += 2;
        break;
        default:
            cerr << "Not implemented, opcode 0x" << hex << opcode << endl;
            exit (EXIT_FAILURE);
    }
}

void CPU::_9_opcode(CPU *cpu, unsigned short opcode){
    // 9xy0 - SNE Vx, Vy
    // Skip next instruction if Vx != Vy.
    // The values of Vx and Vy are compared, 
    // and if they are not equal, the program counter is increased by 2.

    unsigned short x = (opcode & 0x0F00) >> 8;
    unsigned short y = (opcode & 0x00F0) >> 4;
    if(cpu->V[x] != cpu->V[y]){
        cpu->pc += 4;
    } else {
        cpu->pc += 2;
    }

}

void CPU::_A_opcode(CPU *cpu, unsigned short opcode){
    // Annn - LD I, addr
    // Set I = nnn.
    // The value of register I is set to nnn.
    cpu->I = opcode & 0x0FFF;
    cpu->pc += 2;
}

void CPU::_B_opcode(CPU *cpu, unsigned short opcode){
    // Bnnn - JP V0, addr
    // Jump to location nnn + V0.
    // The program counter is set to nnn plus the value of V0.
    cpu->pc = cpu->V[0] + (opcode & 0x0FFF);
}

void CPU::_C_opcode(CPU *cpu, unsigned short opcode){
    
    // Cxkk - RND Vx, byte
    // Set Vx = random byte AND kk.
    // The interpreter generates a random number from 0 to 255, 
    // which is then ANDed with the value kk. 
    // The results are stored in Vx. See instruction 8xy2 for more information on AND.
    unsigned short r = rand() % 0xFF;
    unsigned short x = (opcode & 0x0F00) >> 8;
    unsigned short kk = opcode & 0x00FF; 

    cpu->set_reg(x, kk & r);
    cpu->pc += 2;
}

void CPU::_D_opcode(CPU *cpu, unsigned short opcode){
    /*
    Dxyn - DRW Vx, Vy, nibble
    Display n-byte sprite starting at memory location I at (Vx, Vy), 
    set VF = collision.

    The interpreter reads n bytes from memory, starting at the address stored in
    I. These bytes are then displayed as sprites on screen at coordinates (Vx,
    Vy). Sprites are XORed onto the existing screen. If this causes any pixels to
    be erased, VF is set to 1, otherwise it is set to 0. If the sprite is
    positioned so part of it is outside the coordinates of the display, it wraps
    around to the opposite side of the screen.
    */
    unsigned char vx = cpu->V[(opcode & 0x0F00) >> 8];
    unsigned char vy = cpu->V[(opcode & 0x00F0) >> 4];
    unsigned short n = (opcode & 0x000F);
    //printf("v[%x]=%x, v[%x]=%x\n",(opcode & 0x0F00) >> 8, vx, (opcode & 0x00F0) >> 4, vy);
    //if(vy >= 32) {
    //    vy = vy % 32;
    //}
    //if(vx >= 64) {
    //    vx = vx % 64;
    //}

    cpu->set_reg(0xF, 0); // we start assuming no collisions
    // Remember that one pixel is 1byte (encoded in 8 bit)
    
    for(int y=0; y<n; y++){
        unsigned char pixel = cpu->memManager.get_mem_at(cpu->I + y);
        for(int x=0; x<8; x++){
            // checking bit by bit the pixel (0x80 == 128)
            // If the and is zero, then we don't care as the sprite
            // is doing nothing (if screen was zero it stays to zero and if
            // it was one it stays to one -> try out a xor to see).
            
            if( (pixel & (0x80 >> x)) != 0){
               
                if(cpu->gfx.get_at(vx+x, vy+y) == 1) {
                    // collision: we are flipping out a bit
                    cpu->set_reg(0xF, 1);
                    printf("Flipping out\n");
                }
                // 0x1 is the bit at 1 in the pixel -- rememebr we ignored the zeros
                unsigned char res = cpu->gfx.get_at(vx+x, vy+y) ^ 0x1; 
                cpu->gfx.set_at(vx+x, vy+y, res);
            }
        }
    }
    cpu->gfx.set_draw(true);    
    cpu->pc += 2;

}

void CPU::_E_opcode(CPU *cpu, unsigned short opcode){
   
    unsigned char vx = cpu->V[(opcode & 0x0F00) >> 8]; 
    switch(opcode & 0x00FF){
        case 0x009E:
            // EX9E  KeyOp if(key()==Vx) Skips the next instruction
            // if the key stored in VX is pressed. 
            
            if(cpu->keys.get_key_at(vx) != 0){
                cpu-> pc += 4;
            } else {
                 cpu-> pc += 2;
            }
        break;
        case 0x00A1:
            //Skips the next instruction 
            // if the key stored in VX isn't pressed.
        //printf("keys[%x]: %d",vx, cpu->keys.get_key_at(vx));
            if(cpu->keys.get_key_at(vx) == 0){
                cpu-> pc += 4;
            } else {
                 cpu-> pc += 2;
            }
        break;
    }
}

void CPU::_F_opcode(CPU *cpu, unsigned short opcode){
    unsigned short x = (opcode & 0X0F00) >> 8;
    switch(opcode & 0x00FF){
        case 0x0007:
            // Sets VX to the value of the delay timer.
            cpu->set_reg(x, cpu->delay_timer);
            cpu->pc += 2;
        break;
        case 0x000A:
            // A key press is awaited, and then stored in VX.
            for(int i=0; i < cpu->keys.get_size(); i++){
                if(cpu->keys.get_key_at(cpu->V[x]) != 0){
                    cpu->set_reg(x, i);
                    cpu->pc += 2;
                    break;
                }
            }
            // We do not increment pc and try again
        break;
        case 0x0015:
            // Sets the delay timer to VX.
            cpu->delay_timer = cpu->V[x];
            cpu->pc += 2;
        break;
        case 0x0018:
            // Sets the sound timer to VX.
            cpu->sound_timer = cpu->V[x];
            cpu->pc += 2;
        break;
        case 0x001E:
            // The values of I and Vx are added, 
            // and the results are stored in I.
            // VF is set to 1 when there is a range overflow (I+VX>0xFFF), 
            // and to 0 when there isn't.
            if(cpu->I + cpu->V[x] > 0xFFF) {
                cpu->V[0xF] = 1;
            } else {
                cpu->V[0xF] = 0;
            }

            cpu->I += cpu->V[x];
            cpu->pc += 2;
        break;
        case 0x0029:
            // Sets I to the location of the sprite for the character in VX. 
            // Characters 0-F (in hexadecimal) are represented by a 4x5 font.
            cpu->I = cpu->V[x]*0x5;
            cpu->pc += 2;
        break;
        case 0x0033:
        
            /*
            Stores the binary-coded decimal representation of VX, with the most
            significant of three digits at the address in I, the middle digit at I
            plus 1, and the least significant digit at I plus 2. (In other words,
            take the decimal representation of VX, place the hundreds digit in
            memory at location in I, the tens digit at location I+1, and the ones
            digit at location I+2.)
            */
            cpu->memManager.set_mem(cpu->I, cpu->V[x] / 100);
            cpu->memManager.set_mem(cpu->I+1, (cpu->V[x] / 10) % 10);
            cpu->memManager.set_mem(cpu->I+2, (cpu->V[x] % 100) %10);
            cpu->pc += 2;
        break;
        case 0x0055:
            // Stores V0 to VX (including VX) in memory starting at address I. 
            // I is increased by 1 for each value written.
            for(int i=0; i<=x; i++) {
                cpu->memManager.set_mem(cpu->I, cpu->V[i]);
                cpu->I += 1;
            }
            cpu->pc += 2;
            
        break;
        case 0x0065:
            // Fills V0 to VX (including VX) with values from memory starting at address I. 
            // I is increased by 1 for each value written.
            for(int i=0; i<=x; i++) {
                cpu->set_reg(i, cpu->memManager.get_mem_at(cpu->I));
                cpu->I += 1;
            }
            cpu->pc += 2;
        break;
        default:
            cerr << "Not implemented, opcode [0xF000] 0x" << hex << opcode << endl;
            //exit (EXIT_FAILURE);
    }
}



