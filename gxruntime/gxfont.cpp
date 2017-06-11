#include "gxfont.h"
#include "gxgraphics.h"
#include "gxcanvas.h"

gxFont::gxFont(
	gxGraphics *graphics, gxCanvas *canvas,
	int width, int height, int begin_char, int end_char, int def_char,
	int *offs, int *widths) {

}
gxFont::~gxFont() {

}

int gxFont::charWidth(int c)const {
	return 0;
}
void gxFont::render(gxCanvas *dest, unsigned color_argb, int x, int y, const std::string &t) {

}

//ACCESSORS
int gxFont::getWidth()const {
	return 0;
}
int gxFont::getHeight()const {
	return 0;
}
int gxFont::getWidth(const std::string &text)const {
	return 0;
}
bool gxFont::isPrintable(int chr)const {
	return false;
}
