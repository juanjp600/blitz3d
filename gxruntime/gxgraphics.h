#ifndef GXGRAPHICS_H
#define GXGRAPHICS_H

#include <irrlicht.h>
#include <set>

class gxRuntime;
class gxScene;
class gxCanvas;
class gxMovie;
class gxFont;
class gxRuntime;
class gxMesh;

class gxGraphics {
public:
	gxRuntime *runtime;

	irr::IrrlichtDevice* irrDevice;
	irr::video::IVideoDriver* irrDriver;
	bool sceneOpen = false;
	irr::video::IShaderConstantSetCallBack* flipShaderCallback;
	irr::video::E_MATERIAL_TYPE quad2dMaterialType;
	irr::scene::IMeshSceneNode* flipQuad; irr::scene::IMesh* flipMesh;
	irr::scene::IMeshBuffer* flipBuf;

	bool isRunning();
private:
	int w,h,d,flags;

	gxCanvas *back_canvas;
	gxCanvas *currRenderCanvas;

	std::set<gxCanvas*> canvas_set;
	std::set<gxFont*> font_set;

	gxFont *def_font;

	bool running;

	gxGraphics(int inW, int inH, int inD, int inFlags);
	~gxGraphics();
public:
	static gxGraphics* open(int inW, int inH, int inD, int inFlags);
	void close();

	void cleanup();
	void resize(int inW,int inH);


	enum{
		GRAPHICS_WINDOWED=1,	//windowed mode
		GRAPHICS_SCALED=2,		//scaled window
		GRAPHICS_3D=4,			//when wouldn't you use 3d mode
		GRAPHICS_AUTOSUSPEND=8	//suspend graphics when app suspended
	};
	//MANIPULATORS
	void setRenderCanvas( gxCanvas* canvas );
	void flip( bool vwait );

	//SPECIAL!
	void copy( gxCanvas *dest,int dx,int dy,int dw,int dh,gxCanvas *src,int sx,int sy,int sw,int sh );

	//ACCESSORS
	int getWidth()const;
	int getHeight()const;

	gxCanvas *getBackCanvas();
	gxFont *getDefaultFont()const;

	//OBJECTS
	gxCanvas *createCanvas( int width,int height,int flags );
	gxCanvas *loadCanvas( const std::string &file,int flags );
	gxCanvas *verifyCanvas( gxCanvas *canvas );
	void freeCanvas( gxCanvas *canvas );

	gxMovie *openMovie( const std::string &file,int flags );
	gxMovie *verifyMovie( gxMovie *movie );
	void closeMovie( gxMovie *movie );

	gxFont *loadFont( const std::string &font,int height,int flags );
	gxFont *verifyFont( gxFont *font );
	void freeFont( gxFont *font );

	gxScene *createScene( int flags );
	gxScene *verifyScene( gxScene *scene );
	void freeScene( gxScene *scene );

	gxMesh *createMesh( int max_verts,int max_tris,int flags );
	gxMesh *verifyMesh( gxMesh *mesh );
	void freeMesh( gxMesh *mesh );
};

#endif
