#include "gxdevice.h"

gxDevice::gxDevice() {

}
gxDevice::~gxDevice() {

}

void gxDevice::reset() {

}
void gxDevice::downEvent(int key) {

}
void gxDevice::upEvent(int key) {

}
void gxDevice::setDownState(int key, bool down) {

}

void gxDevice::flush() {

}

bool gxDevice::keyDown(int key) {
	return false;
}

int gxDevice::keyHit(int key) {
	return 0;
}

int gxDevice::getKey() {
	return 0;
}

float gxDevice::getAxisState(int axis) {
	return 0.0f;
}
