#ifndef GXGRAPHICS_H
#define GXGRAPHICS_H

#include <set>

class gxRuntime;
class gxCanvas;
class gxMovie;
class gxFont;
class gxRuntime;

class gxGraphics {
public:
	gxRuntime *runtime;
private:
	gxCanvas *front_canvas,*back_canvas;

	std::set<gxCanvas*> canvas_set;
public:
	//SPECIAL!
	void copy( gxCanvas *dest,int dx,int dy,int dw,int dh,gxCanvas *src,int sx,int sy,int sw,int sh );

	//ACCESSORS
	int getWidth()const;
	int getHeight()const;
	
	gxCanvas *getFrontCanvas()const;
	gxCanvas *getBackCanvas()const;

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