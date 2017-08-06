#include "Main.h"

int main() {
	Main* main = new Main();
	while (main->run()) {}
	return 0;
}

Main::Main() {
	device = irr::createDevice(irr::video::EDT_DIRECT3D11,irr::core::dimension2du(1280,720),32,false,false,false);
}

bool Main::run() {
	return device->run();
}
