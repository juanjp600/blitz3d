#include "gxgraphics.h"
#include "gxcanvas.h"
#include "gxfont.h"
#include "MainEventReceiver.h"

#include <windows.h>

const char* QUAD2D_SHADER_CODE =
	"cbuffer cbParams : register(b0)\n\
	{\n\
	\n\
	};\n\
	\n\
	struct VS_INPUT\n\
	{\n\
		float4 vPosition : POSITION;\n\
		float3 vNormal   : NORMAL;\n\
	    float2 texCoord0 : TEXCOORD0;\n\
	};\n\
	\n\
	// Vertex shader output structure\n\
	struct VS_OUTPUT\n\
	{\n\
		float4 Position   : SV_Position;	// vertex position\n\
	    float2 TexCoord0  : TEXTURE0;		// tex coords\n\
	};\n\
	\n\
	VS_OUTPUT vertexMain( VS_INPUT input )\n\
	{\n\
		VS_OUTPUT Output;\n\
		\n\
		Output.Position = input.vPosition;\n\
		\n\
		Output.TexCoord0 = input.texCoord0;\n\
		Output.TexCoord0.y = input.texCoord0.y;\n\
		\n\
		return Output;\n\
	}\n\
	\n\
	Texture2D imgTexture : register(t0);\n\
	\n\
	SamplerState st0 : register(s0);\n\
	\n\
	float4 pixelMain( VS_OUTPUT input ) : SV_Target\n\
	{\n\
		float4 Output;\n\
		\n\
	    float4 col0 = imgTexture.Sample( st0, input.TexCoord0 ).bgra;\n\
		\n\
		Output = col0;\n\
		\n\
		return Output;\n\
	}\n";

class FlipShaderCallback : public irr::video::IShaderConstantSetCallBack
{
public:
	virtual void OnSetConstants(irr::video::IMaterialRendererServices* services,
		irr::s32 userData) { }
};

void gxGraphics::resize(int inW,int inH) {
	w = inW; h = inH;
	if (sceneOpen) {
		irrDriver->endScene();
		sceneOpen = false;
	}

	cleanup();
	delete back_canvas;

	irrDevice->setWindowSize(irr::core::dimension2d<irr::u32>(w,h));
	irrDevice->run();

	back_canvas = new gxCanvas(this,w,h,gxCanvas::CANVAS_TEX_VIDMEM);
	flipMesh->getMeshBuffer(0)->getMaterial().setTexture(0,back_canvas->getRenderTex());
	flipQuad->getMesh()->getMeshBuffer(0)->getMaterial().setTexture(0,back_canvas->getRenderTex());
	flipQuad->setMaterialTexture(0,back_canvas->getRenderTex());
	setRenderCanvas(back_canvas);
	flip(false);
}

void gxGraphics::resizeCanvas(gxCanvas* canvas,int inW, int inH) {
	if (!sceneOpen) {
		irrDriver->beginScene(false,false);
		sceneOpen = true;
	}
	irrDriver->setRenderTarget(canvas->getRenderTex(),false,false);
	setDefaultMaterial();
	irrDriver->draw2DRectangle(irr::video::SColor(255,255,0,255),irr::core::recti(0,0,inW,inH));
	irrDriver->draw2DImage(canvas->getIrrTex(),irr::core::recti(0,0,inW,inH),irr::core::recti(0,0,canvas->getWidth(),canvas->getHeight()));
	irrDriver->setRenderTarget(currRenderCanvas->getRenderTex(),false,false);
	//irrDriver->endScene();
	sceneOpen = false;
}

void gxGraphics::setDefaultMaterial() {
	irr::video::SMaterial mat;
	mat.MaterialType = irr::video::EMT_SOLID;
	irrDriver->setMaterial(mat);
}

gxGraphics* gxGraphics::open(int inW, int inH, int inD, int inFlags) {
	gxGraphics* graphics = new gxGraphics(inW,inH,inD,inFlags);

	while (!graphics->irrDevice->run()) {}

	irr::video::IGPUProgrammingServices* gpu = graphics->irrDriver->getGPUProgrammingServices();

	graphics->flipShaderCallback = new FlipShaderCallback();

	graphics->quad2dMaterialType = (irr::video::E_MATERIAL_TYPE)gpu->addHighLevelShaderMaterial(
		QUAD2D_SHADER_CODE,"vertexMain",irr::video::EVST_VS_4_0,
		QUAD2D_SHADER_CODE,"pixelMain",irr::video::EPST_PS_4_0,graphics->flipShaderCallback);

	graphics->flipShaderCallback->drop();

	graphics->currRenderCanvas = 0;

	graphics->back_canvas = new gxCanvas(graphics,inW,inH,gxCanvas::CANVAS_TEX_VIDMEM);
	graphics->setRenderCanvas(graphics->back_canvas);
	irr::video::IVertexDescriptor* vDesc = graphics->irrDriver->getVertexDescriptor(0);

	irr::scene::SMesh* mesh = new irr::scene::SMesh();
	irr::scene::CMeshBuffer<irr::video::S3DVertex>* buf = new irr::scene::CMeshBuffer<irr::video::S3DVertex>(vDesc);

	irr::video::S3DVertex verts[4];
	verts[0]=irr::video::S3DVertex(irr::core::vector3df(-1.f,-1.f,0.f),irr::core::vector3df(0.f,0.f,1.f),irr::video::SColor(255,255,255,255),irr::core::vector2df(0.f,1.f)); //top left
	verts[1]=irr::video::S3DVertex(irr::core::vector3df(1.f,-1.f,0.f),irr::core::vector3df(0.f,0.f,1.f),irr::video::SColor(255,255,255,255),irr::core::vector2df(1.f,1.f)); //top right
	verts[2]=irr::video::S3DVertex(irr::core::vector3df(-1.f,1.f,0.f),irr::core::vector3df(0.f,0.f,1.f),irr::video::SColor(255,255,255,255),irr::core::vector2df(0.f,0.f)); //bottom left
	verts[3]=irr::video::S3DVertex(irr::core::vector3df(1.f,1.f,0.f),irr::core::vector3df(0.f,0.f,1.f),irr::video::SColor(255,255,255,255),irr::core::vector2df(1.f,0.f)); //bottom right

	irr::scene::CVertexBuffer<irr::video::S3DVertex>* vertexBuffer = new irr::scene::CVertexBuffer<irr::video::S3DVertex>();
	irr::scene::CIndexBuffer* indexBuffer = new irr::scene::CIndexBuffer(irr::video::EIT_16BIT);

	for (unsigned int j = 0; j<4; ++j) {
		vertexBuffer->addVertex(verts[j]);
	}

	indexBuffer->addIndex(1);
	indexBuffer->addIndex(0);
	indexBuffer->addIndex(2);
	indexBuffer->addIndex(2);
	indexBuffer->addIndex(3);
	indexBuffer->addIndex(1);

	buf->setVertexBuffer(vertexBuffer, 0);
	buf->setIndexBuffer(indexBuffer);

	buf->getMaterial().MaterialType = graphics->quad2dMaterialType;
	buf->getMaterial().BackfaceCulling = true;
	buf->getMaterial().setTexture(0,graphics->back_canvas->getRenderTex());

	mesh->addMeshBuffer(buf);
	buf->drop();

	graphics->flipMesh = mesh;
	graphics->flipQuad = graphics->irrDevice->getSceneManager()->addMeshSceneNode(mesh);

	graphics->flip(false);

	return graphics;
}

gxGraphics::gxGraphics(int inW, int inH, int inD, int inFlags) {
	w = inW; h = inH; d = 32; flags = inFlags;

	MainEventReceiver* eventReceiver = new MainEventReceiver();
	irrDevice = irr::createDevice(irr::video::EDT_DIRECT3D11,irr::core::dimension2d<irr::u32>(w,h),32,false,false,false,eventReceiver);
	irrDriver = irrDevice->getVideoDriver();

	running = true;

	sceneOpen = false;

	getGfxModes();
}

void gxGraphics::getGfxModes() {
	irr::video::IVideoModeList* modeList = irrDevice->getVideoModeList();
	for (int i=0; i<modeList->getVideoModeCount(); i++) {
		if (modeList->getVideoModeDepth(i) != 32) continue;
		gfxModes.push_back(modeList->getVideoModeResolution(i));
	}
}

int gxGraphics::getGfxModeCount() {
	return gfxModes.size();
}

int gxGraphics::getGfxModeWidth(int mode) {
	return gfxModes[mode].Width;
}

int gxGraphics::getGfxModeHeight(int mode) {
	return gfxModes[mode].Height;
}

void gxGraphics::cleanup() {
	while( font_set.size() ) freeFont( *font_set.begin() );
	while( canvas_set.size() ) freeCanvas( *canvas_set.begin() );
}

void gxGraphics::close() {
	if (sceneOpen) {
		irrDriver->endScene();
		sceneOpen = false;
	}

	flipQuad->remove();
	flipMesh->drop();

	cleanup();

	printf("CLOSING DEVICE\n");
	irrDevice->closeDevice();
	while (irrDevice->run()) {}
	delete this;
}

gxGraphics::~gxGraphics() {
	irrDevice->drop();
}

//MANIPULATORS
void gxGraphics::setRenderCanvas(gxCanvas* canvas) {
	if (!sceneOpen) {
		irrDriver->beginScene(false,false);
		sceneOpen = true;
	}
	if (currRenderCanvas) currRenderCanvas->cleanupRenderTex();
	currRenderCanvas = canvas;
	irrDriver->setRenderTarget(canvas->getRenderTex(),false,false);
}

void gxGraphics::flip(bool vwait) {
	if (!sceneOpen) {
		irrDriver->beginScene(false,false);
		sceneOpen = true;
	}
	irrDriver->setRenderTarget(0,true,true,irr::video::SColor(255,100,60,0));
	flipQuad->render();
	irrDriver->endScene(); sceneOpen = false;
	running = irrDevice->run();
	setRenderCanvas(currRenderCanvas);
}

bool gxGraphics::isRunning() {
	return running;
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

gxCanvas *gxGraphics::getBackCanvas() {
	return back_canvas;
}
gxFont *gxGraphics::getDefaultFont()const {
	return 0;
}

//OBJECTS
gxCanvas *gxGraphics::createCanvas(int width, int height, int flags) {
	gxCanvas* newCanvas = new gxCanvas(this,width,height,flags);
	canvas_set.insert(newCanvas);
	return newCanvas;
}
gxCanvas *gxGraphics::loadCanvas(const std::string &file, int flags) {
	gxCanvas* newCanvas = new gxCanvas(this,file,flags);
	canvas_set.insert(newCanvas);
	return newCanvas;
}
gxCanvas *gxGraphics::verifyCanvas(gxCanvas *canvas) {
	if (canvas == back_canvas && !!back_canvas) return back_canvas;
	return canvas_set.count(canvas) ? canvas : 0;
}
void gxGraphics::freeCanvas(gxCanvas *canvas) {
	printf("DELETING CANVAS\n");
	delete canvas;
	canvas_set.erase(canvas);
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
	gxFont* newFont = new gxFont(this,font.c_str(),height);
	font_set.insert(newFont);
	return newFont;
}
gxFont *gxGraphics::verifyFont(gxFont *font) {
	return font_set.count(font) ? font : 0;
}
void gxGraphics::freeFont(gxFont *font) {
	delete font;
	font_set.erase(font);
}

gxScene *gxGraphics::createScene(int flags) {
	return new gxScene(flags);
}
gxScene *gxGraphics::verifyScene(gxScene *scene) {
	return 0;
}
void gxGraphics::freeScene(gxScene *scene) {
	delete scene;
}

gxMesh *gxGraphics::createMesh(int max_verts, int max_tris, int flags) {
	return 0;
}
gxMesh *gxGraphics::verifyMesh(gxMesh *mesh) {
	return 0;
}
void gxGraphics::freeMesh(gxMesh *mesh) {

}
