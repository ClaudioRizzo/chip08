#include <vector>

namespace devices {
	class AbstractDevice {

	};

	class Keyboard : AbstractDevice {
	private:
		const static int KEYS = 16;
		std::vector<unsigned char> keys;
	public:
		Keyboard();
		unsigned char get_key_at(int key);
		void set_key_at(int key, unsigned char value);
		int get_size();
	};

	class Screen : AbstractDevice {
	private:
		const static int W = 64;
		const static int H = 32;
		std::vector<unsigned char> gfx;
		bool draw_flag;

	public:
		Screen();
		void clearScreen();
		unsigned char get_at(unsigned short x, unsigned short y);
		void set_at(unsigned short x, unsigned short y, unsigned char value);
		bool is_draw();
		void set_draw(bool draw_flag);
	};
}

