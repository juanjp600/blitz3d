
#ifndef GXINPUT_H
#define GXINPUT_H

#include "gxdevice.h"

class gxRuntime;

class gxInput{
public:
	gxRuntime *runtime;
	
	gxInput( gxRuntime *runtime );
	~gxInput();

	void reset();
	bool acquire();
	void unacquire();

private:

	/***** GX INTERFACE *****/
public:
	enum{
		ASC_HOME=1,ASC_END=2,ASC_INSERT=3,ASC_DELETE=4,
		ASC_PAGEUP=5,ASC_PAGEDOWN=6,
		ASC_UP=28,ASC_DOWN=29,ASC_RIGHT=30,ASC_LEFT=31
	};

	void moveMouse( int x,int y );

	gxDevice *getMouse()const;
	gxDevice *getKeyboard()const;
	gxDevice *getJoystick( int port )const;
	int getJoystickType( int port )const;
	int numJoysticks()const;
	int toAscii( int key )const;
};

#endif
