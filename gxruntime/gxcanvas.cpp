#include "gxcanvas.h"
#include "gxgraphics.h"
#include "gxfont.h"

gxCanvas::gxCanvas(gxGraphics* gfx,int inW, int inH, int inFlags) {
	w = inW; h = inH; flags = inFlags;
	irrTex = gfx->irrDriver->addRenderTargetTexture(irr::core::dimension2d<irr::u32>(w,h),"rtcanvas",irr::video::ECF_A8R8G8B8);
	graphics = gfx;
}

void gxCanvas::reset() {
	graphics->irrDriver->removeTexture(irrTex);
	irrTex = graphics->irrDriver->addRenderTargetTexture(irr::core::dimension2d<irr::u32>(w,h),"rtcanvas",irr::video::ECF_A8R8G8B8);
}

gxCanvas::~gxCanvas() {
	graphics->irrDriver->removeTexture(irrTex);
}

irr::video::ITexture* gxCanvas::getIrrTex() {
	return irrTex;
}

//MANIPULATORS
void gxCanvas::setFont(gxFont *inFont) {
	font = inFont;
}
void gxCanvas::setMask(unsigned argb) {

}
void gxCanvas::setColor(unsigned argb) {
	color = irr::video::SColor(255,(argb>>16)&255,(argb>>8)&255,argb&255);
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
	graphics->irrDriver->draw2DLine(irr::core::vector2di(x,y),irr::core::vector2di(x2,y2),color);
}
void gxCanvas::rect(int x, int y, int w, int h, bool solid) {
	graphics->irrDriver->draw2DRectangle(color,irr::core::recti(x,y,x+w,y+h));
}
void gxCanvas::oval(int x, int y, int w, int h, bool solid) {

}
void gxCanvas::text(int x, int y, const std::string &t) {
	font->render(this,color.getAlpha()<<24|color.getRed()<<16|color.getGreen()<<8|color.getBlue(),x,y,t);
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
