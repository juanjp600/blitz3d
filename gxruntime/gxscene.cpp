#include "gxscene.h"

gxScene::gxScene(int flags) {

}

void gxScene::setDither(bool enable) {

}
void gxScene::setAntialias(bool enable) {

}
void gxScene::setWireframe(bool enable) {

}
void gxScene::setFlippedTris(bool enable) {

}
void gxScene::setAmbient(const float rgb[3]) {

}
void gxScene::setAmbient2(const float rgb[3]) {

}
void gxScene::setFogColor(const float rgb[3]) {

}
void gxScene::setFogRange(float nr, float fr) {

}
void gxScene::setFogMode(int mode) {

}
void gxScene::setZMode(int mode) {

}
void gxScene::setViewport(int x, int y, int w, int h) {

}
void gxScene::setOrthoProj(float nr, float fr, float nr_w, float nr_h) {

}
void gxScene::setPerspProj(float nr, float fr, float nr_w, float nr_h) {

}
void gxScene::setViewMatrix(const gxScene::Matrix *matrix) {

}
void gxScene::setWorldMatrix(const gxScene::Matrix *matrix) {

}
void gxScene::setRenderState(const gxScene::RenderState &state) {

}

//rendering
bool gxScene::begin(const std::vector<gxLight*> &lights) {
	return true;
}
void gxScene::clear(const float rgb[3], float alpha, float z, bool clear_argb, bool clear_z) {

}
void gxScene::render(gxMesh *mesh, int first_vert, int vert_cnt, int first_tri, int tri_cnt) {

}
void gxScene::end() {

}

void gxScene::setZMode() {

}
void gxScene::setAmbient() {

}
void gxScene::setFogMode() {

}

//lighting
gxLight *gxScene::createLight(int flags) {
	return 0;
}
void gxScene::freeLight(gxLight *l) {

}

