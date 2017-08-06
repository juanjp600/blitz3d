#ifndef BBIDE_MAIN_H
#define BBIDE_MAIN_H

#include <irrlicht.h>
class Main {
	private:
		irr::IrrlichtDevice* device;
	public:
		Main();

		bool run();
};

#endif //BBIDE_MAIN_H