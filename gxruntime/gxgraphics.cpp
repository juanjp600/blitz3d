#include "gxgraphics.h"

//MANIPULATORS
void gxGraphics::flip(bool vwait) {

}

//SPECIAL!
void gxGraphics::copy(gxCanvas *dest, int dx, int dy, int dw, int dh, gxCanvas *src, int sx, int sy, int sw, int sh) {

}

//ACCESSORS
int gxGraphics::getWidth()const {
	return 0;
}
int gxGraphics::getHeight()const {
	return 0;
}

gxCanvas *gxGraphics::getFrontCanvas()const {
	return 0;
}
gxCanvas *gxGraphics::getBackCanvas()const {
	return 0;
}
gxFont *gxGraphics::getDefaultFont()const {
	return 0;
}

//OBJECTS
gxCanvas *gxGraphics::createCanvas(int width, int height, int flags) {
	return 0;
}
gxCanvas *gxGraphics::loadCanvas(const std::string &file, int flags) {
	return 0;
}
gxCanvas *gxGraphics::verifyCanvas(gxCanvas *canvas) {
	return 0;
}
void gxGraphics::freeCanvas(gxCanvas *canvas) {

}

gxMovie *gxGraphics::openMovie(const std::string &file, int flags) {
	return 0;
}
gxMovie *gxGraphics::verifyMovie(gxMovie *movie) {
	return 0;
}
void gxGraphics::closeMovie(gxMovie *movie) {

}

gxFont *gxGraphics::loadFont(const std::string &font, int height, int flags) {
	return 0;
}
gxFont *gxGraphics::verifyFont(gxFont *font) {
	return 0;
}
void gxGraphics::freeFont(gxFont *font) {

}

gxScene *gxGraphics::createScene(int flags) {
	return 0;
}
gxScene *gxGraphics::verifyScene(gxScene *scene) {
	return 0;
}
void gxGraphics::freeScene(gxScene *scene) {

}

gxMesh *gxGraphics::createMesh(int max_verts, int max_tris, int flags) {
	return 0;
}
gxMesh *gxGraphics::verifyMesh(gxMesh *mesh) {
	return 0;
}
void gxGraphics::freeMesh(gxMesh *mesh) {

}
