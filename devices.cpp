#include <stdexcept>
#include "devices.h"


namespace devices {
	Keyboard::Keyboard() {
		this -> keys.resize(KEYS);
	}

	Screen::Screen() {
		this -> gfx.resize(W*H);
		set_draw(true);
	}

	void Screen::clearScreen() {
		for(int i=0; i<W; i++) {
			for(int j=0; j<H; j++) {
				this -> gfx[i + j] = 0;
			}
		}
		set_draw(true);
	}

	unsigned char Screen::get_at(unsigned short x, unsigned short y){
		return this -> gfx[x + y*W];
	}

	void Screen::set_at(unsigned short x, unsigned short y, unsigned char value){
		this->gfx[x + y*W] = value;
	}

	bool Screen::is_draw(){
		return this->draw_flag;
	}

	void Screen::set_draw(bool draw_flag){
		this->draw_flag = draw_flag;
	}

	unsigned char Keyboard::get_key_at(int key){
		if(key > KEYS - 1) {
			throw std::runtime_error("Key out of bound"+std::to_string(key));
		}
		
		return this->keys[key];
	}

	void Keyboard::set_key_at(int key, unsigned char value){
		this->keys[key] = value;
	}

	int Keyboard::get_size() { return KEYS; }
}