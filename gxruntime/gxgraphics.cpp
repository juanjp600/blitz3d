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
	"	 \n"\
	//"	 Output = (float4(1.0,0.0,0.0,1.0)+col0)*float4(0.5,0.5,0.5,0.5);\n"\

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

	back_canvas = 0;

	flipQuad = 0;
}

gxGraphics::~gxGraphics()
{
	irrDriver->removeAllTextures();
	irrDevice->closeDevice();
}

//MANIPULATORS
void gxGraphics::flip(bool vwait) {
	irrDriver->setRenderTarget(0,true,true,irr::video::SColor(255,100,50,0));
	flipQuad->render();
	irrDriver->endScene();
	irrDevice->run();
	irrDriver->beginScene(false,false);
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
	if (!back_canvas)
	{
		back_canvas = new gxCanvas(this,w,h,gxCanvas::CANVAS_TEX_RGB);

		irrDriver->beginScene(false,false);
		irrDriver->setRenderTarget(back_canvas->getIrrTex(),true,true,irr::video::SColor(255,0,0,0));

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
	return 0;
}
gxCanvas *gxGraphics::verifyCanvas(gxCanvas *canvas) {
	if (canvas == back_canvas && !!back_canvas) return canvas;
	return canvas_set.count(canvas) ? canvas : 0;
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
