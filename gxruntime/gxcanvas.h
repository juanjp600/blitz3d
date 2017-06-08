#ifndef GXCANVAS_H
#define GXCANVAS_H

#include "gxscene.h"

class gxGraphics;
class gxFont;

class gxCanvas {
private:
	int flags;
	gxGraphics *graphics;
public:
	enum{
		CANVAS_TEX_RGB=			0x0001,
		CANVAS_TEX_ALPHA=		0x0002,
		CANVAS_TEX_MASK=		0x0004,
		CANVAS_TEX_MIPMAP=		0x0008,
		CANVAS_TEX_CLAMPU=		0x0010,
		CANVAS_TEX_CLAMPV=		0x0020,
		CANVAS_TEX_SPHERE=		0x0040,
		//CANVAS_TEX_CUBE=		0x0080,
		CANVAS_TEX_VIDMEM=		0x0100,
		CANVAS_TEX_HICOLOR=		0x0200,

		CANVAS_TEXTURE=			0x10000,
		CANVAS_NONDISPLAY=		0x20000,
		CANVAS_HIGHCOLOR=		0x40000
	};
	//MANIPULATORS
	void setFont( gxFont *font );
	void setMask( unsigned argb );
	void setColor( unsigned argb );
	void setClsColor( unsigned argb );
	void setOrigin( int x,int y );
	void setHandle( int x,int y );
	void setViewport( int x,int y,int w,int h );

	void cls();
	void plot( int x,int y );
	void line( int x,int y,int x2,int y2 );
	void rect( int x,int y,int w,int h,bool solid );
	void oval( int x,int y,int w,int h,bool solid );
	void text( int x,int y,const std::string &t );
	void blit( int x,int y,gxCanvas *src,int src_x,int src_y,int src_w,int src_h,bool solid );

	bool collide( int x,int y,const gxCanvas *src,int src_x,int src_y,bool solid )const;
	bool rect_collide( int x,int y,int rect_x,int rect_y,int rect_w,int rect_h,bool solid )const;

	void setPixel( int x,int y,unsigned argb );
	void copyPixel( int x,int y,gxCanvas *src,int src_x,int src_y );
	unsigned getPixel( int x,int y )const;

	//ACCESSORS
	int getWidth()const;
	int getHeight()const;
	int getDepth()const;
	int getFlags()const{ return flags; }
	void getOrigin( int *x,int *y )const;
	void getHandle( int *x,int *y )const;
	void getViewport( int *x,int *y,int *w,int *h )const;
	unsigned getMask()const;
	unsigned getColor()const;
	unsigned getClsColor()const;
};

#endif
