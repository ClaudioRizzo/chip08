#include <string>
#include <vector>
#include <map>
#include <iostream>

#include "MMU.h"
#include "devices.h"



class CPU {
typedef void (*OpcodeFunction)(CPU*, unsigned short);
private:
	
	MMU memManager;
	devices::Keyboard keys;
	devices::Screen gfx;
	
	const static int REGISTERS = 16;
	std::vector<unsigned char> V;
	unsigned short I; // Index register 
	
	unsigned char delay_timer;
	unsigned char sound_timer;

	unsigned short current_opcode;
	void set_reg(int reg, unsigned char value);

	static void _0_opcode(CPU *cpu, unsigned short opcode);
	static void _1_opcode(CPU *cpu, unsigned short opcode);
	static void _2_opcode(CPU *cpu, unsigned short opcode);
	static void _3_opcode(CPU *cpu, unsigned short opcode);
	static void _4_opcode(CPU *cpu, unsigned short opcode);
	static void _5_opcode(CPU *cpu, unsigned short opcode);
	static void _6_opcode(CPU *cpu, unsigned short opcode);
	static void _7_opcode(CPU *cpu, unsigned short opcode);
	static void _8_opcode(CPU *cpu, unsigned short opcode);
	static void _9_opcode(CPU *cpu, unsigned short opcode);
	static void _A_opcode(CPU *cpu, unsigned short opcode);
	static void _B_opcode(CPU *cpu, unsigned short opcode);
	static void _C_opcode(CPU *cpu, unsigned short opcode);
	static void _D_opcode(CPU *cpu, unsigned short opcode);
	static void _E_opcode(CPU *cpu, unsigned short opcode);
	static void _F_opcode(CPU *cpu, unsigned short opcode);
	
	const std::map<unsigned short, OpcodeFunction> opcode_table =
	{
		{0x0000, &_0_opcode},
		{0x1000, &_1_opcode},
		{0x2000, &_2_opcode},
		{0x3000, &_3_opcode},
		{0x4000, &_4_opcode},
		{0x5000, &_5_opcode},
		{0x6000, &_6_opcode},
		{0x7000, &_7_opcode},
		{0x8000, &_8_opcode},
		{0x9000, &_9_opcode},
		{0xA000, &_A_opcode},
		{0xB000, &_B_opcode},
		{0xC000, &_C_opcode},
		{0xD000, &_D_opcode},
		{0xE000, &_E_opcode},
		{0xF000, &_F_opcode}
	};




public:
	CPU();
	~CPU();
	void loadProgram(std::string pathToProgram);
	void execute_opcode(unsigned short opcode);
	unsigned short fetch_opcode();
	devices::Screen& get_graphic();
	devices::Keyboard& get_keyboard();
	unsigned short pc; // program counter
	void debug();
	void update_delay_timer();
	void update_sound_timer();
	

};
