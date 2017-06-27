#include "gxgraphics.h"
#include "gxcanvas.h"

class MainEventReceiver : public irr::IEventReceiver {
private:
	bool errorState;
	std::string errorStr;
public:
	virtual bool OnEvent(const irr::SEvent& event) {
		if (event.EventType == irr::EET_LOG_TEXT_EVENT) {
			switch (event.LogEvent.Level) {
				case irr::ELL_ERROR:
					printf(errorStr.c_str()); printf("\n\n");
					errorStr+=event.LogEvent.Text;
					errorStr+="\n\n";
					errorState = true;
					break;
				default:

					break;
			}
		}
	}
	bool getErrorState() { return errorState; }
	std::string getErrorStr() { return errorStr; }
};

const char FLIP_SHADER_CODE[] =
	"cbuffer cbParams : register(b0)\n"\
	"{\n"\
	"\n"\
	"};\n"\
	"\n"\
	"struct VS_INPUT\n"\
	"{\n"\
	"	float4 vPosition : POSITION;\n"\
	"	float3 vNormal   : NORMAL;\n"\
	"    float2 texCoord0 : TEXCOORD0;\n"\
	"};\n"\
	"\n"\
	"// Vertex shader output structure\n"\
	"struct VS_OUTPUT\n"\
	"{\n"\
	"	float4 Position   : SV_Position;	// vertex position\n"\
	"    float2 TexCoord0  : TEXTURE0;		// tex coords\n"\
	"};\n"\
	"\n"\
	"VS_OUTPUT vertexMain( VS_INPUT input )\n"\
	"{\n"\
	"	VS_OUTPUT Output;\n"\
	"\n"\
	"	Output.Position = input.vPosition;\n"\
	"\n"\
	"	Output.TexCoord0 = input.texCoord0;\n"\
	"	Output.TexCoord0.y = -input.texCoord0.y;\n"\
	"	\n"\
	"	return Output;\n"\
	"}\n"\
	"\n"\
	"Texture2D imgTexture : register(t0);\n"\
	"\n"\
	"SamplerState st0 : register(s0);\n"\
	"	\n"\
	"float4 pixelMain( VS_OUTPUT input ) : SV_Target\n"\
	"{ \n"\
	"	float4 Output;\n"\
	"\n"\
	"    float4 col0 = imgTexture.Sample( st0, input.TexCoord0 ).rgba;\n"\
	"	\n"\
	"	 Output = col0;\n"\
	"    Output[3] = 1.0;\n"\
	"	\n"\
	"	return Output;\n"\
	"}\n";

class FlipShaderCallback : public irr::video::IShaderConstantSetCallBack
{
public:
	virtual void OnSetConstants(irr::video::IMaterialRendererServices* services,
		irr::s32 userData) { }
};

gxGraphics::gxGraphics(int inW, int inH, int inD, int inFlags) {
	w = inW; h = inH; d = 32; flags = inFlags;

	irrDevice = irr::createDevice(irr::video::EDT_DIRECT3D11,irr::core::dimension2d<irr::u32>(w,h),32,false);
	irrDevice->setWindowCaption(L"IrrlichtDevice");
	irrDriver = irrDevice->getVideoDriver();

	irr::video::IGPUProgrammingServices* gpu = irrDriver->getGPUProgrammingServices();
	flipShaderCallback = new FlipShaderCallback();

	flipMaterialType = (irr::video::E_MATERIAL_TYPE)gpu->addHighLevelShaderMaterial(FLIP_SHADER_CODE,"vertexMain",irr::video::EVST_VS_5_0,FLIP_SHADER_CODE,"pixelMain",irr::video::EPST_PS_5_0,flipShaderCallback);

	back_canvas = new gxCanvas(irrDriver,w,h,gxCanvas::CANVAS_TEX_RGB);

	//NOTE: red and blue are swapped for background color
	irrDriver->setRenderTarget(back_canvas->getIrrTex(),true,true,irr::video::SColor(255,40,60,0));

	irrDriver->draw2DRectangle(irr::video::SColor(255,5,0,255),irr::core::recti(300,300,600,600));
	//NOTE: draw2DRectangleOutline is BROKEN
	irrDriver->setRenderTarget(0,false,false);

	irr::scene::SMesh* mesh = new irr::scene::SMesh();
	irr::scene::CMeshBuffer<irr::video::S3DVertex>* buf = new irr::scene::CMeshBuffer<irr::video::S3DVertex>(irrDriver->getVertexDescriptor(0));

	irr::video::S3DVertex verts[4];
	verts[0]=irr::video::S3DVertex(irr::core::vector3df(-1.f,-1.f,0.f),irr::core::vector3df(0.f,0.f,1.f),irr::video::SColor(255,255,255,255),irr::core::vector2df(0.f,0.f)); //top left
	verts[1]=irr::video::S3DVertex(irr::core::vector3df(1.f,-1.f,0.f),irr::core::vector3df(0.f,0.f,1.f),irr::video::SColor(255,255,255,255),irr::core::vector2df(1.f,0.f)); //top right
	verts[2]=irr::video::S3DVertex(irr::core::vector3df(-1.f,1.f,0.f),irr::core::vector3df(0.f,0.f,1.f),irr::video::SColor(255,255,255,255),irr::core::vector2df(0.f,1.f)); //bottom left
	verts[3]=irr::video::S3DVertex(irr::core::vector3df(1.f,1.f,0.f),irr::core::vector3df(0.f,0.f,1.f),irr::video::SColor(255,255,255,255),irr::core::vector2df(1.f,1.f)); //bottom right
	
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
	
	buf->getMaterial().MaterialType = flipMaterialType;
	buf->getMaterial().BackfaceCulling = true;
	buf->getMaterial().setTexture(0,back_canvas->getIrrTex());

	mesh->addMeshBuffer(buf);

	flipQuad = irrDevice->getSceneManager()->addMeshSceneNode(mesh);
}

gxGraphics::~gxGraphics()
{
	irrDriver->removeAllTextures();
	irrDevice->closeDevice();
}

//MANIPULATORS
void gxGraphics::flip(bool vwait) {
	irrDriver->beginScene(true,true);
	flipQuad->render();
	irrDriver->endScene();
	irrDevice->run();
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
	return 0; //there's no good reason to try accessing the front canvas directly, so I won't allow it
}
gxCanvas *gxGraphics::getBackCanvas()const {
	return back_canvas;
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
	if (canvas == back_canvas) return canvas;
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
	return new gxScene(flags);
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
