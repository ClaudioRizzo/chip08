#include <iostream>
#include <vector>
#include <stdexcept>
#include <string>
#include "MMU.h"


MMU::MMU() {
	
	memory.resize(MEM_SIZE);
	stack.resize(STACK_SIZE);
	sp = 0;

	// load fontset
	for(int i=0; i<80; i++) {
		set_mem(i, chip8_fontset[i]);
	}
}

unsigned short MMU::pop(){
	if(sp <= 0) {
		// empty stack
		return -1;
	}
	unsigned short addr = stack[sp];
	sp -= 1;
	return addr;

}

void MMU::push(unsigned short address){
	if(sp == STACK_SIZE - 1) {
		throw std::runtime_error("Stack overflow, sp @ "+std::to_string(sp));	
	}
	stack[sp+1] = address;
	sp += 1;
}

unsigned char MMU::get_mem_at(int mem_loc){
	return this -> memory[mem_loc];
}

int MMU::get_real_mem_size(){
	return MEM_SIZE - 512;
}

void MMU::set_mem(int address, unsigned char value) {
	this -> memory[address] = value;
	
}

int MMU::get_mem_start(){
	return 0x200;
}