
#ifndef GXINPUT_H
#define GXINPUT_H

#include <irrlicht.h>
#include <string>
//#include "gxdevice.h"

class gxRuntime;
class MainEventReceiver;

class gxInput{
public:
	gxRuntime *runtime;
	irr::IrrlichtDevice* irrDevice;
	MainEventReceiver* eventReceiver;

	gxInput( gxRuntime *runtime );
	~gxInput();

private:

	/***** GX INTERFACE *****/
public:
	enum{
		ASC_HOME=1,ASC_END=2,ASC_INSERT=3,ASC_DELETE=4,
		ASC_PAGEUP=5,ASC_PAGEDOWN=6,
		ASC_UP=28,ASC_DOWN=29,ASC_RIGHT=30,ASC_LEFT=31
	};

	void moveMouse( int x,int y );
	int getMouseX();
	int getMouseY();
	bool mouseDown(int button);
	int mouseHit(int button);
	void flushMouse();

	int getKey();
	bool keyDown(int key);
	int keyHit(int key);
	std::string textInput(const std::string &prev);
	void flushKeys();
};

#endif
