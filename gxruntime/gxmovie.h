
#ifndef GXMOVIE_H
#define GXMOVIE_H

#include "std.h"
#include "gxcanvas.h"

class gxGraphics;

class gxMovie{

public:
	gxMovie( gxGraphics *gfx );
	~gxMovie();

private:
	bool playing;
	RECT src_rect;
	gxGraphics *gfx;
	gxCanvas *canvas;
	
	/***** GX INTERFACE *****/
public:

	bool draw( gxCanvas *dest,int x,int y,int w,int h );

	bool isPlaying()const{ return playing; }
	int getWidth()const{ return src_rect.right; }
	int getHeight()const{ return src_rect.bottom; }
};

#endif
