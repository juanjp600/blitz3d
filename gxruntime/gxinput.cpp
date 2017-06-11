#include "gxinput.h"

gxInput::gxInput(gxRuntime *rntime) {
	runtime = rntime;
}
gxInput::~gxInput() {

}

void gxInput::reset() {

}
bool gxInput::acquire() {
	return false;
}
void gxInput::unacquire() {
	
}

void gxInput::moveMouse(int x, int y) {

}

gxDevice *gxInput::getMouse()const {
	return 0;
}
gxDevice *gxInput::getKeyboard()const {
	return 0;
}
gxDevice *gxInput::getJoystick(int port)const {
	return 0;
}
int gxInput::getJoystickType(int port)const {
	return 0;
}
int gxInput::numJoysticks()const {
	return 0;
}
int gxInput::toAscii(int key)const {
	return 0;
}
