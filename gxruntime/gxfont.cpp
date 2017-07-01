#include "gxfont.h"
#include "gxgraphics.h"
#include "gxcanvas.h"

gxFont::gxFont( gxGraphics *inGraphics, const char* filename, int size ) {
	graphics = inGraphics;
	internalFont = irr::gui::CGUITTFont::create(graphics->irrDevice,/*graphics->quad2dMaterialType,*/irr::io::path(filename),size);

}
gxFont::~gxFont() {
	internalFont->drop();
}

int gxFont::charWidth(int c)const {
	return internalFont->getCharDimension(L'T').Width;
}
void gxFont::render(gxCanvas *dest, unsigned color_argb, int x, int y, const std::string &t) {
	irr::core::dimension2d<irr::u32> dims = internalFont->getDimension(t.c_str());

	irr::video::SColor color(color_argb>>24,(color_argb>>16)&255,(color_argb>>8)&255,color_argb&255);

	internalFont->draw(irr::core::stringw(t.c_str()),irr::core::recti(x,y,x+dims.Width,y+dims.Height),color);
}

//ACCESSORS
int gxFont::getWidth()const {
	return 0;
}
int gxFont::getHeight()const {
	return internalFont->getFontSize();
}
int gxFont::getWidth(const std::string &text)const {
	return internalFont->getDimension(text.c_str()).Width;
}
bool gxFont::isPrintable(int chr)const {
	return false;
}
