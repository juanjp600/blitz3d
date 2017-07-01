
#ifndef GXFONT_H
#define GXFONT_H

#include <string>

#include <irrlicht.h>
#include "CGUITTFont.h"

class gxCanvas;
class gxGraphics;

class gxFont{
public:
	gxFont( gxGraphics *inGraphics,const char* filename,int size );
	~gxFont();

	int charWidth( int c )const;
	void render( gxCanvas *dest,unsigned color_argb,int x,int y,const std::string &t );

private:
	gxGraphics *graphics;
	//int width,height,begin_char,end_char,def_char;
	//int *offs,*widths;

	irr::gui::CGUITTFont* internalFont;

	/***** GX INTERFACE *****/
public:
	enum{
		FONT_BOLD=1,
		FONT_ITALIC=2,
		FONT_UNDERLINE=4
	};

	//ACCESSORS
	int getWidth()const;							//width of widest char
	int getHeight()const;							//height of font
	int getWidth( const std::string &text )const;	//width of string
	bool isPrintable( int chr )const;				//printable char?
};

#endif
