#include "cpu.h"
#include <iostream>
#include <iomanip>
#include <SDL.h>
#include <stdio.h>

#include <chrono>
#include <ctime>
#include <thread>

//Screen dimension constants
const int modifier = 10;
const int SCREEN_WIDTH = 64 * modifier;
const int SCREEN_HEIGHT = 32 * modifier;

using namespace std;

void printScreenCli(CPU& e) {
	if(e.get_graphic().is_draw()){
		for (int y = 0; y < SCREEN_HEIGHT/modifier; y++) {
			for(int x=0; x < SCREEN_WIDTH/modifier; x++) {
				if(e.get_graphic().get_at(x, y) != 0){
					printf("%d", e.get_graphic().get_at(x, y));
				} else {
					printf("%d", e.get_graphic().get_at(x, y));
				}
			}
			printf("\n");
		}
	}

}

void printScreen(CPU& emu, SDL_Renderer *renderer){
	if(emu.get_graphic().is_draw()) {
		for (int y = 0; y < SCREEN_HEIGHT/modifier; y++) {
			for(int x=0; x < SCREEN_WIDTH/modifier; x++) {
				
				if(emu.get_graphic().get_at(x,y) != 0) {
					SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
					SDL_RenderDrawPoint(renderer, x, y);
					
				} else {
					SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
					SDL_RenderDrawPoint(renderer, x, y);
				}
				SDL_RenderSetScale(renderer, 10,10);
			}
			
		}

    	SDL_RenderPresent(renderer);
    }
    emu.get_graphic().set_draw(false);
}

void keyboardDown(char code, CPU& emu) {
	devices::Keyboard& keys = emu.get_keyboard();
	switch( code ){
		case SDLK_1:
			keys.set_key_at(0x1, 1);
		break;
		case SDLK_2:
			keys.set_key_at(0x2, 1);
		break;
		case SDLK_3:
			keys.set_key_at(0x3, 1);
		break;
		case SDLK_4:
			keys.set_key_at(0xC, 1);
		break;
		case SDLK_q:
			keys.set_key_at(0x4, 1);
		break;
		case SDLK_w:
			keys.set_key_at(0x5, 1);
		break;
		case SDLK_e:
			keys.set_key_at(0x6, 1);
		break;
		case SDLK_r:
			keys.set_key_at(0xD, 1);
		break;
		case SDLK_a:
			keys.set_key_at(0x7, 1);
		break;
		case SDLK_s:
			keys.set_key_at(0x8, 1);
		break;
		case SDLK_d:
			keys.set_key_at(0x9, 1);
		break;
		case SDLK_z:
			keys.set_key_at(0xA, 1);
		break;
		case SDLK_x:
			keys.set_key_at(0x0, 1);
		break;
		case SDLK_c:
			keys.set_key_at(0xB, 1);
		break;
		case SDLK_v:
			keys.set_key_at(0xF, 1);
		break;
		default:
		break;
	}
}

void keyboardUp(char code, CPU& emu) {
	devices::Keyboard& keys = emu.get_keyboard();
	switch( code ){
		case SDLK_1:
			keys.set_key_at(0x1, 0);
		break;
		case SDLK_2:
			keys.set_key_at(0x2, 0);
		break;
		case SDLK_3:
			keys.set_key_at(0x3, 0);
		break;
		case SDLK_4:
			keys.set_key_at(0xC, 0);
		break;
		case SDLK_q:
			keys.set_key_at(0x4, 0);
		break;
		case SDLK_w:
			keys.set_key_at(0x5, 0);
		break;
		case SDLK_e:
			keys.set_key_at(0x6, 0);
		break;
		case SDLK_r:
			keys.set_key_at(0xD, 0);
		break;
		case SDLK_a:
			keys.set_key_at(0x7, 0);
		break;
		case SDLK_s:
			keys.set_key_at(0x8, 0);
		break;
		case SDLK_d:
			keys.set_key_at(0x9, 0);
		break;
		case SDLK_z:
			keys.set_key_at(0xA, 0);
		break;
		case SDLK_x:
			keys.set_key_at(0x0, 0);
		break;
		case SDLK_c:
			keys.set_key_at(0xB, 0);
		break;
		case SDLK_v:
			keys.set_key_at(0xF, 0);
		break;
		default:
		break;
	}
}

int main() {
	
	CPU emulator = CPU();
	emulator.loadProgram("/home/clod/Downloads/puzzle.rom");


    bool quit = false;
    
    SDL_Event event;
    SDL_Renderer *renderer;
    SDL_Window *window;
    
    SDL_Init(SDL_INIT_VIDEO);
    SDL_CreateWindowAndRenderer(SCREEN_WIDTH, SCREEN_HEIGHT, 0, &window, &renderer);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderClear(renderer);
    
    
    //emulator.get_graphic().set_at(0,0, 'x');
	
    
	while(!quit) {
		// execute a cycle
		unsigned short opcode = emulator.fetch_opcode();
		emulator.execute_opcode(opcode);
		//printf("executed: 0x%x\n", opcode);
	
		emulator.update_delay_timer();
		emulator.update_sound_timer();

		// Render the screen
		printScreen(emulator, renderer);
	    //printScreenCli(emulator);
		// check inputs
	    
    	
    	while(SDL_PollEvent(&event)) {
    		switch(event.type){
    			case SDL_QUIT:
    				quit=true;
    			break;
    			case SDL_KEYDOWN:
                	keyboardDown(event.key.keysym.sym, emulator);
                break;
    			case SDL_KEYUP:
    				keyboardUp(event.key.keysym.sym, emulator);
    			break;
    			default: break;
    		}
    	}
    	
    	//if (SDL_PollEvent(&event) && event.type == SDL_QUIT) {
    	//	quit = true;
    	//}
    	chrono::milliseconds timespan(2); // or whatever
		this_thread::sleep_for(timespan);

       
	}

	SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return EXIT_SUCCESS;
   
}