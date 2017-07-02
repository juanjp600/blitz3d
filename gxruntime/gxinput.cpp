#include "gxinput.h"
#include "gxgraphics.h"
#include "gxruntime.h"
#include "MainEventReceiver.h"

gxInput::gxInput(gxRuntime *rntime) {
	runtime = rntime;
	irrDevice = runtime->graphics->irrDevice;
	eventReceiver = (MainEventReceiver*)irrDevice->getEventReceiver();

}
gxInput::~gxInput() {

}

int gxInput::getMouseX() {
	return eventReceiver->getMousePos().X;
}

int gxInput::getMouseY() {
	return eventReceiver->getMousePos().Y;
}

bool gxInput::mouseDown(int button) {
	return eventReceiver->getMouseDown(button);
}

int gxInput::mouseHit(int button) {
	return eventReceiver->getMouseHit(button);
}

void gxInput::flushMouse() {
	eventReceiver->clearMouse();
}

static irr::EKEY_CODE blitzToIrrKeyCode(int code) {
	switch (code) {
		case 1:
			return irr::KEY_ESCAPE;
			break;
		case 59:
			return irr::KEY_F1;
			break;
		case 60:
			return irr::KEY_F2;
			break;
		case 61:
			return irr::KEY_F3;
			break;
		case 62:
			return irr::KEY_F4;
			break;
		case 63:
			return irr::KEY_F5;
			break;
		case 64:
			return irr::KEY_F6;
			break;
		case 65:
			return irr::KEY_F7;
			break;
		case 66:
			return irr::KEY_F8;
			break;
		case 67:
			return irr::KEY_F9;
			break;
		case 68:
			return irr::KEY_F10;
			break;
		case 87:
			return irr::KEY_F11;
			break;
		case 88:
			return irr::KEY_F12;
			break;
		case 197:
			return irr::KEY_PAUSE;
			break;
		case 2:
			return irr::KEY_KEY_1;
			break;
		case 3:
			return irr::KEY_KEY_2;
			break;
		case 4:
			return irr::KEY_KEY_3;
			break;
		case 5:
			return irr::KEY_KEY_4;
			break;
		case 6:
			return irr::KEY_KEY_5;
			break;
		case 7:
			return irr::KEY_KEY_6;
			break;
		case 8:
			return irr::KEY_KEY_7;
			break;
		case 9:
			return irr::KEY_KEY_8;
			break;
		case 10:
			return irr::KEY_KEY_9;
			break;
		case 11:
			return irr::KEY_KEY_0;
			break;
		case 12:
			return irr::KEY_MINUS;
			break;
		case 14:
			return irr::KEY_BACK;
			break;
		case 210:
			return irr::KEY_INSERT;
			break;
		case 199:
			return irr::KEY_HOME;
			break;
		case 201:
			return irr::KEY_PRIOR;
			break;
		case 69:
			return irr::KEY_NUMLOCK;
			break;
		case 15:
			return irr::KEY_TAB;
			break;
		case 16:
			return irr::KEY_KEY_Q;
			break;
		case 17:
			return irr::KEY_KEY_W;
			break;
		case 18:
			return irr::KEY_KEY_E;
			break;
		case 19:
			return irr::KEY_KEY_R;
			break;
		case 20:
			return irr::KEY_KEY_T;
			break;
		case 21:
			return irr::KEY_KEY_Y;
			break;
		case 22:
			return irr::KEY_KEY_U;
			break;
		case 23:
			return irr::KEY_KEY_I;
			break;
		case 24:
			return irr::KEY_KEY_O;
			break;
		case 25:
			return irr::KEY_KEY_P;
			break;
		case 211:
			return irr::KEY_DELETE;
			break;
		case 207:
			return irr::KEY_END;
			break;
		case 209:
			return irr::KEY_NEXT;
			break;
		case 71:
			return irr::KEY_NUMPAD7;
			break;
		case 72:
			return irr::KEY_NUMPAD8;
			break;
		case 73:
			return irr::KEY_NUMPAD9;
			break;
		case 30:
			return irr::KEY_KEY_A;
			break;
		case 31:
			return irr::KEY_KEY_S;
			break;
		case 32:
			return irr::KEY_KEY_D;
			break;
		case 33:
			return irr::KEY_KEY_F;
			break;
		case 34:
			return irr::KEY_KEY_G;
			break;
		case 35:
			return irr::KEY_KEY_H;
			break;
		case 36:
			return irr::KEY_KEY_J;
			break;
		case 37:
			return irr::KEY_KEY_K;
			break;
		case 38:
			return irr::KEY_KEY_L;
			break;
		case 75:
			return irr::KEY_NUMPAD4;
			break;
		case 76:
			return irr::KEY_NUMPAD5;
			break;
		case 77:
			return irr::KEY_NUMPAD6;
			break;

		case 42:
			return irr::KEY_LSHIFT;
			break;
		case 44:
			return irr::KEY_KEY_Z;
			break;
		case 45:
			return irr::KEY_KEY_X;
			break;
		case 46:
			return irr::KEY_KEY_C;
			break;
		case 47:
			return irr::KEY_KEY_V;
			break;
		case 48:
			return irr::KEY_KEY_B;
			break;
		case 49:
			return irr::KEY_KEY_N;
			break;
		case 50:
			return irr::KEY_KEY_M;
			break;
		case 51:
			return irr::KEY_COMMA;
			break;
		case 52:
			return irr::KEY_PERIOD;
			break;
		case 54:
			return irr::KEY_RSHIFT;
			break;
		case 200:
			return irr::KEY_UP;
			break;
		case 79:
			return irr::KEY_NUMPAD1;
			break;
		case 80:
			return irr::KEY_NUMPAD2;
			break;
		case 81:
			return irr::KEY_NUMPAD3;
			break;
		case 29:
			return irr::KEY_LCONTROL;
			break;
		case 56:
			return irr::KEY_MENU;
			break;
		case 57:
			return irr::KEY_SPACE;
			break;
		case 157:
			return irr::KEY_RCONTROL;
			break;
		case 203:
			return irr::KEY_LEFT;
			break;
		case 208:
			return irr::KEY_DOWN;
			break;
		case 205:
			return irr::KEY_RIGHT;
			break;
		case 82:
			return irr::KEY_NUMPAD0;
			break;
		case 83:
			return irr::KEY_DECIMAL;
			break;
	}
	return irr::KEY_KEY_CODES_COUNT;
}

static int irrToBlitzKeyCode(irr::EKEY_CODE code) {
	switch (code) {
		case irr::KEY_ESCAPE:
			return 1;
			break;
		case irr::KEY_F1:
			return 59;
			break;
		case irr::KEY_F2:
			return 60;
			break;
		case irr::KEY_F3:
			return 61;
			break;
		case irr::KEY_F4:
			return 62;
			break;
		case irr::KEY_F5:
			return 63;
			break;
		case irr::KEY_F6:
			return 64;
			break;
		case irr::KEY_F7:
			return 65;
			break;
		case irr::KEY_F8:
			return 66;
			break;
		case irr::KEY_F9:
			return 67;
			break;
		case irr::KEY_F10:
			return 68;
			break;
		case irr::KEY_F11:
			return 87;
			break;
		case irr::KEY_F12:
			return 88;
			break;
			//case irr::KEY_PRINTSCREEN:
			//	return 183;
			//	break;
			//case irr::KEY_SCROLLLOCK:
			//	return 70;
			break;
		case irr::KEY_PAUSE:
			return 197;
			break;
			//case irr::KEY_LEFTAPOST:
			//	return 86;
			//	break;
		case irr::KEY_KEY_1:
			return 2;
			break;
		case irr::KEY_KEY_2:
			return 3;
			break;
		case irr::KEY_KEY_3:
			return 4;
			break;
		case irr::KEY_KEY_4:
			return 5;
			break;
		case irr::KEY_KEY_5:
			return 6;
			break;
		case irr::KEY_KEY_6:
			return 7;
			break;
		case irr::KEY_KEY_7:
			return 8;
			break;
		case irr::KEY_KEY_8:
			return 9;
			break;
		case irr::KEY_KEY_9:
			return 10;
			break;
		case irr::KEY_KEY_0:
			return 11;
			break;
		case irr::KEY_MINUS:
			return 12;
			break;
			//case irr::KEY_EQUALS:
			//	return 13;
			//	break;
		case irr::KEY_BACK:
			return 14;
			break;
		case irr::KEY_INSERT:
			return 210;
			break;
		case irr::KEY_HOME:
			return 199;
			break;
		case irr::KEY_PRIOR:
			return 201;
			break;
		case irr::KEY_NUMLOCK:
			return 69;
			break;
			//case irr::KEY_NUMRIGHTSLASH:
			//	return 181;
			//	break;
			//case irr::KEY_NUMTIMES:
			//	return 55;
			//	break;
			//case irr::KEY_NUMMINUS:
			//	return 74;
			//	break;

		case irr::KEY_TAB:
			return 15;
			break;
		case irr::KEY_KEY_Q:
			return 16;
			break;
		case irr::KEY_KEY_W:
			return 17;
			break;
		case irr::KEY_KEY_E:
			return 18;
			break;
		case irr::KEY_KEY_R:
			return 19;
			break;
		case irr::KEY_KEY_T:
			return 20;
			break;
		case irr::KEY_KEY_Y:
			return 21;
			break;
		case irr::KEY_KEY_U:
			return 22;
			break;
		case irr::KEY_KEY_I:
			return 23;
			break;
		case irr::KEY_KEY_O:
			return 24;
			break;
		case irr::KEY_KEY_P:
			return 25;
			break;
			//case irr::KEY_LEFTSQUARE:
			//	return 26;
			//	break;
			//case irr::KEY_RIGHTSQUARE:
			//	return 27;
			//	break;
			//case irr::KEY_ENTERMAIN:
			//	return 28;
			//	break;
		case irr::KEY_DELETE:
			return 211;
			break;
		case irr::KEY_END:
			return 207;
			break;
		case irr::KEY_NEXT:
			return 209;
			break;
		case irr::KEY_NUMPAD7:
			return 71;
			break;
		case irr::KEY_NUMPAD8:
			return 72;
			break;
		case irr::KEY_NUMPAD9:
			return 73;
			break;
			//case irr::KEY_NUMPLUS:
			//	return 78;
			//	break;

			//case irr::KEY_CAPS:
			//	return 58;
			//	break;
		case irr::KEY_KEY_A:
			return 30;
			break;
		case irr::KEY_KEY_S:
			return 31;
			break;
		case irr::KEY_KEY_D:
			return 32;
			break;
		case irr::KEY_KEY_F:
			return 33;
			break;
		case irr::KEY_KEY_G:
			return 34;
			break;
		case irr::KEY_KEY_H:
			return 35;
			break;
		case irr::KEY_KEY_J:
			return 36;
			break;
		case irr::KEY_KEY_K:
			return 37;
			break;
		case irr::KEY_KEY_L:
			return 38;
			break;
			//case irr::KEY_COLON:
			//	return 39;
			//	break;
			//case irr::KEY_RIGHTAPOST:
			//	return 40;
			//	break;
			//case irr::KEY_HASH:
			//	return 41;
			//	break;
		case irr::KEY_NUMPAD4:
			return 75;
			break;
		case irr::KEY_NUMPAD5:
			return 76;
			break;
		case irr::KEY_NUMPAD6:
			return 77;
			break;

		case irr::KEY_LSHIFT:
			return 42;
			break;
			//case irr::KEY_LEFTSLASH:
			//	return 43;
			//	break;
		case irr::KEY_KEY_Z:
			return 44;
			break;
		case irr::KEY_KEY_X:
			return 45;
			break;
		case irr::KEY_KEY_C:
			return 46;
			break;
		case irr::KEY_KEY_V:
			return 47;
			break;
		case irr::KEY_KEY_B:
			return 48;
			break;
		case irr::KEY_KEY_N:
			return 49;
			break;
		case irr::KEY_KEY_M:
			return 50;
			break;
		case irr::KEY_COMMA:
			return 51;
			break;
		case irr::KEY_PERIOD:
			return 52;
			break;
			//case irr::KEY_RIGHTSLASH:
			//	return 53;
			//	break;
		case irr::KEY_RSHIFT:
			return 54;
			break;
		case irr::KEY_UP:
			return 200;
			break;
		case irr::KEY_NUMPAD1:
			return 79;
			break;
		case irr::KEY_NUMPAD2:
			return 80;
			break;
		case irr::KEY_NUMPAD3:
			return 81;
			break;
			//case irr::KEY_NUMENTER:
			//	return 156;
			//	break;

		case irr::KEY_LCONTROL:
			return 29;
			break;
			//case irr::KEY_LEFTWIN:
			//	return 219;
			//	break;
		case irr::KEY_MENU:
			return 56;
			break;
		case irr::KEY_SPACE:
			return 57;
			break;
			//case irr::KEY_ALTGR:
			//	return 184;
			//	break;
			//case irr::KEY_RIGHTWIN:
			//	return 220;
			//	break;
			//case irr::KEY_MENU:
			//	return 221;
			//	break;
		case irr::KEY_RCONTROL:
			return 157;
			break;
		case irr::KEY_LEFT:
			return 203;
			break;
		case irr::KEY_DOWN:
			return 208;
			break;
		case irr::KEY_RIGHT:
			return 205;
			break;
		case irr::KEY_NUMPAD0:
			return 82;
			break;
		case irr::KEY_DECIMAL:
			return 83;
			break;
	}
	return 0;
}

int gxInput::getKey() {
	return irrToBlitzKeyCode(eventReceiver->getLastKeyHit());
}

bool gxInput::keyDown(int key) {
	return eventReceiver->getKeyDown(blitzToIrrKeyCode(key));
}

int gxInput::keyHit(int key) {
	return eventReceiver->getKeyHit(blitzToIrrKeyCode(key));
}

std::string gxInput::textInput(const std::string &prev) {
	return irr::core::stringc(eventReceiver->getCharQueue(irr::core::stringw(prev.c_str())).c_str()).c_str();
}

void gxInput::flushKeys() {
	eventReceiver->clearKeys();
}

/*void gxInput::reset() {

}
bool gxInput::acquire() {
	return false;
}
void gxInput::unacquire() {
	
}*/

void gxInput::moveMouse(int x, int y) {
	irrDevice->getCursorControl()->setPosition(irr::core::vector2di(x,y));
}

/*gxDevice *gxInput::getMouse()const {
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
}*/
