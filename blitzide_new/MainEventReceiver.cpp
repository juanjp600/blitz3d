#include "MainEventReceiver.h"

#include <iostream>

Input::Input(Input::DEVICE inDevice,irr::EKEY_CODE inCode) {
    device = inDevice;
    if (device == DEVICE::KEY) {
        keyCode = inCode;
    } else {
        std::cout<<"WARNING: received EKEY_CODE for non-key device\n";
    }
}

Input::Input(Input::DEVICE inDevice,unsigned char inCode) {
    device = inDevice;
    if (device == DEVICE::KEY) {
        std::cout<<"WARNING: received unsigned char for key device\n";
    } else if (device == DEVICE::MOUSE) {
        mouseButton = inCode;
    } else {
        std::cout<<"WARNING: can't assign unsigned char to this input device!\n";
    }
}

bool Input::operator<(const Input& other) const {
    if (device == other.device) {
        switch (device) {
            case DEVICE::KEY:
                return keyCode<other.keyCode;
            break;
            case DEVICE::MOUSE:
                return mouseButton<other.mouseButton;
            break;
        }
    }
    return device < other.device;
}

MainEventReceiver::MainEventReceiver() {
        memset(keyDown, false, sizeof(keyDown));
        memset(mouseDown, false, sizeof(mouseDown));
		memset(keyHit,0,sizeof(keyHit));
		memset(mouseHit,0,sizeof(mouseHit));
        MousePosition = irr::core::position2di(0,0);
        errorStr = ""; errorState = false;
}

bool MainEventReceiver::OnEvent(const irr::SEvent& event) {
    if (event.EventType == irr::EET_KEY_INPUT_EVENT) {
		if (!keyDown[event.KeyInput.Key]) { keyHit[event.KeyInput.Key]++; lastKeyHit = event.KeyInput.Key; }
        keyDown[event.KeyInput.Key] = event.KeyInput.PressedDown;
        if (event.KeyInput.PressedDown) {
            //std::cout<<event.KeyInput.Key<<"\n";
            wchar_t newChar = event.KeyInput.Char;
            if ((int)newChar>=32 || (int)newChar==8 || (int)newChar==10/*LF*/ || (int)newChar==13/*CR*/ || (int)newChar==9/*TAB*/) {
                charQueue+=newChar;
                while (charQueue.size()>8) {
                    charQueue.erase(0);
                }
            }
        }
        return true;
    } else if (event.EventType == irr::EET_MOUSE_INPUT_EVENT) {
        switch(event.MouseInput.Event) {
        case irr::EMIE_LMOUSE_PRESSED_DOWN:
			if (!mouseDown[0]) { mouseHit[0]++; }
            mouseDown[0] = true;
            break;

        case irr::EMIE_RMOUSE_PRESSED_DOWN:
			if (!mouseDown[1]) { mouseHit[1]++; }
            mouseDown[1] = true;
            break;

        case irr::EMIE_LMOUSE_LEFT_UP:
            mouseDown[0] = false;
            break;

        case irr::EMIE_RMOUSE_LEFT_UP:
            mouseDown[1] = false;
            break;

        case irr::EMIE_MMOUSE_PRESSED_DOWN:
			if (!mouseDown[2]) { mouseHit[2]++; }
            mouseDown[2] = true;
            break;

        case irr::EMIE_MMOUSE_LEFT_UP:
            mouseDown[2] = false;
            break;

        case irr::EMIE_MOUSE_MOVED:
            MousePosition.X = event.MouseInput.X;
            MousePosition.Y = event.MouseInput.Y;
            break;

        case irr::EMIE_MOUSE_WHEEL:
            MouseWheel = event.MouseInput.Wheel;
            break;

        default:

            break;
        }
        return true;
    } else if (event.EventType == irr::EET_LOG_TEXT_EVENT) {
        switch (event.LogEvent.Level) {
        case irr::ELL_ERROR:
            std::cout<<errorStr.c_str()<<"\n\n";
            errorStr+=event.LogEvent.Text;
            errorStr+="\n\n";
            errorState = true;
            break;
        default:

            break;
        }
    }
    return false;
}

bool MainEventReceiver::getKeyDown(irr::EKEY_CODE keyCode) const {
    return keyDown[keyCode];
}
int MainEventReceiver::getKeyHit(irr::EKEY_CODE keyCode) {
	int kh = keyHit[keyCode]; keyHit[keyCode]=0;
    return kh;
}
irr::EKEY_CODE MainEventReceiver::getLastKeyHit() {
	irr::EKEY_CODE kh = lastKeyHit; lastKeyHit = irr::KEY_KEY_CODES_COUNT;
	return kh;
}
bool MainEventReceiver::getMouseDown(unsigned char keyCode) const {
    return mouseDown[keyCode];
}
int MainEventReceiver::getMouseHit(unsigned char keyCode) {
	int mh = mouseHit[keyCode]; mouseHit[keyCode]=0;
    return mh;
}
bool MainEventReceiver::getInput(const Input& input) const {
    switch (input.device) {
    case Input::DEVICE::KEY:
        return getKeyDown(input.keyCode);
        break;
    case Input::DEVICE::MOUSE:
        return getMouseDown(input.mouseButton);
        break;
    default:
        std::cout<<"WARNING: can't handle this input device!\n";
        return false;
        break;
    }
}
irr::core::position2di MainEventReceiver::getMousePos() const {
    return MousePosition;
}

float MainEventReceiver::getMouseWheel() {
	float mw = MouseWheel; MouseWheel = 0.f;
    return mw;
}

void MainEventReceiver::clearMouse() {
	memset(mouseDown,false,sizeof(mouseDown));
	memset(mouseHit,0,sizeof(mouseHit));
    MouseWheel = 0.f;
}

void MainEventReceiver::clearKeys() {
	lastKeyHit = irr::KEY_KEY_CODES_COUNT;
	charQueue = "";
	memset(keyDown,false,sizeof(keyDown));
	memset(keyHit,0,sizeof(keyHit));
}

irr::core::stringw MainEventReceiver::getCharQueue(irr::core::stringw in,bool includeSpecialChars) {
    if (charQueue.size()==0) {
        return in;
    }
    irr::core::stringw retVal = in;
    for (int i=0;i<charQueue.size();i++) {
        if (((int)charQueue[i]!=8 && (int)charQueue[i]!=10 && (int)charQueue[i]!=13 && (int)charQueue[i]!=9) || includeSpecialChars) {
            retVal+=charQueue[i];
        } else if ((int)charQueue[i]==8 && retVal.size()>0) {
            retVal.erase(retVal.size()-1);
        }
    }
    charQueue = "";
    return retVal;
}

void MainEventReceiver::clearCharQueue() {
    charQueue = "";
}

bool MainEventReceiver::getErrorState() {
    return errorState;
}

std::string MainEventReceiver::getErrorStr() {
    return errorStr;
}

void MainEventReceiver::resetErrorState() {
    errorStr = ""; errorState = false;
}
