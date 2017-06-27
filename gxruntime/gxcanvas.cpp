#include "gxcanvas.h"

gxCanvas::gxCanvas(irr::video::IVideoDriver* driver,int w, int h, int flags) {
	irrTex = driver->addRenderTargetTexture(irr::core::dimension2d<irr::u32>(w,h),"rtcanvas",irr::video::ECF_A8R8G8B8);
}

irr::video::ITexture* gxCanvas::getIrrTex() {
	return irrTex;
}

//MANIPULATORS
void gxCanvas::setFont(gxFont *font) {

}
void gxCanvas::setMask(unsigned argb) {

}
void gxCanvas::setColor(unsigned argb) {

}
void gxCanvas::setClsColor(unsigned argb) {

}
void gxCanvas::setOrigin(int x, int y) {

}
void gxCanvas::setHandle(int x, int y) {

}
void gxCanvas::setViewport(int x, int y, int w, int h) {

}

void gxCanvas::cls() {

}
void gxCanvas::plot(int x, int y) {

}
void gxCanvas::line(int x, int y, int x2, int y2) {

}
void gxCanvas::rect(int x, int y, int w, int h, bool solid) {

}
void gxCanvas::oval(int x, int y, int w, int h, bool solid) {

}
void gxCanvas::text(int x, int y, const std::string &t) {

}
void gxCanvas::blit(int x, int y, gxCanvas *src, int src_x, int src_y, int src_w, int src_h, bool solid) {

}

bool gxCanvas::collide(int x, int y, const gxCanvas *src, int src_x, int src_y, bool solid)const {
	return false;
}
bool gxCanvas::rect_collide(int x, int y, int rect_x, int rect_y, int rect_w, int rect_h, bool solid)const {
	return false;
}

void gxCanvas::setPixel(int x, int y, unsigned argb) {
	
}
void gxCanvas::copyPixel(int x, int y, gxCanvas *src, int src_x, int src_y) {

}
unsigned gxCanvas::getPixel(int x, int y)const {
	return 0;
}

//ACCESSORS
int gxCanvas::getWidth()const {
	return 0;
}
int gxCanvas::getHeight()const {
	return 0;
}
int gxCanvas::getDepth()const {
	return 32;
}
void gxCanvas::getOrigin(int *x, int *y)const {
	
}
void gxCanvas::getHandle(int *x, int *y)const {

}
void gxCanvas::getViewport(int *x, int *y, int *w, int *h)const {

}
unsigned gxCanvas::getMask()const {
	return 0;
}
unsigned gxCanvas::getColor()const {
	return 0;
}
unsigned gxCanvas::getClsColor()const {
	return 0;
}
