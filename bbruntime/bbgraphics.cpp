
#include "std.h"
#include "bbgraphics.h"
#include "bbinput.h"

#include "../gxruntime/gxcanvas.h"
#include "../gxruntime/gxfont.h"
#include "../gxruntime/gxmovie.h"

gxGraphics *gx_graphics;
gxCanvas *gx_canvas;

struct GfxMode{
	int w,h,d;
};

class bbImage{
public:
	bbImage( const vector<gxCanvas*> &f ):frames(f){
	}
	~bbImage(){
		for( int k=0;k<(int)frames.size();++k ){
			gx_graphics->freeCanvas( frames[k] );
		}
	}
	const vector<gxCanvas*> &getFrames()const{
		return frames;
	}
	void replaceFrame( int n,gxCanvas *c ){
		gx_graphics->freeCanvas( frames[n] );
		frames[n]=c;
	}
private:
	vector<gxCanvas*> frames;
};

//degrees to radians
static const float dtor=0.0174532925199432957692369076848861f;

static bool filter;
static bool auto_dirty;
static bool auto_midhandle;
static set<bbImage*> image_set;
static int curs_x,curs_y;
static gxCanvas *p_canvas;

static gxFont *curr_font;
static unsigned curr_color;
static unsigned curr_clsColor;

static vector<GfxMode> gfx_modes;

static inline bool debugImage( bbImage *i,int frame,const char* a ){
	if( debug ){
		if( !image_set.count(i) ) {
			RTEX( "Image does not exist" );
			return false;
		}
		if( frame>=i->getFrames().size() ) {
			RTEX( "Image frame out of range" );
			return false;
		}
	} else {
		if( !image_set.count(i) ) {
			errorLog.push_back(std::string(a)+std::string(": Image does not exist"));
			return false;
		}
		if( frame>=i->getFrames().size() ) {
			errorLog.push_back(std::string(a)+std::string(": Image frame out of range"));
			return false;
		}
	}
	return true;
}

static inline bool debugFont( gxFont *f,const char* a ){
	if( gx_graphics->verifyFont( f ) ) return true;
	if( debug ){
		RTEX( "Font does not exist" );
	} else {
		errorLog.push_back(std::string(a)+std::string(": Font does not exist"));
	}
	return false;
}

static inline bool debugCanvas( gxCanvas *c,const char* a ){
	if( gx_graphics->verifyCanvas( c ) ) return true;
	if( debug ){
		RTEX( "Buffer does not exist" );
	} else {
		errorLog.push_back(std::string(a)+std::string(": Buffer does not exist"));
	}
	return false;
}

static inline bool debugMode( int n,const char* a ){
	if( debug ){
		if( n<1 || n>gfx_modes.size() ){
			RTEX( "Illegal graphics mode index" );
			return false;
		}
	} else {
		if( n<1 || n>gfx_modes.size() ){
			errorLog.push_back(std::string(a)+std::string(": Illegal graphics mode index"));
			return false;
		}
	}
	return true;
}

void bbFreeImage( bbImage *i );

static void freeGraphics(){
#ifdef PRO
	extern void blitz3d_close();
	blitz3d_close();
#endif
	while( image_set.size() ) bbFreeImage( *image_set.begin() );
	if( p_canvas ){
		gx_graphics->freeCanvas( p_canvas );
		p_canvas=0;
	}
}

#define RED(_X_) ( ((_X_)>>16) & 0xff )
#define GRN(_X_) ( ((_X_)>>8) & 0xff )
#define BLU(_X_) ( (_X_) & 0xff )

static int getPixel( gxCanvas *c,float x,float y ){
	if (!debugCanvas( c,"getPixel" )) return 0;

	x-=.5f;y-=.5f;
	float fx=floor(x),fy=floor(y);
	int ix=(int)fx,iy=(int)fy;fx=x-fx;fy=y-fy;

	int tl=c->getPixel( ix,iy );
	int tr=c->getPixel( ix+1,iy );
	int br=c->getPixel( ix+1,iy+1 );
	int bl=c->getPixel( ix,iy+1 );

	float w1=(1-fx)*(1-fy),w2=fx*(1-fy),w3=(1-fx)*fy,w4=fx*fy;

	float r=RED(tl)*w1+RED(tr)*w2+RED(bl)*w3+RED(br)*w4;
	float g=GRN(tl)*w1+GRN(tr)*w2+GRN(bl)*w3+GRN(br)*w4;
	float b=BLU(tl)*w1+BLU(tr)*w2+BLU(bl)*w3+BLU(br)*w4;

	return (int(r+.5f)<<16)|(int(g+.5f)<<8)|int(b+.5f);
}

struct vec2{ float x,y; };

static vec2 vrot( float m[2][2],const vec2 &v ){
	vec2 t;t.x=m[0][0]*v.x+m[0][1]*v.y;t.y=m[1][0]*v.x+m[1][1]*v.y;
	return t;
}

static float vmin( float a,float b,float c,float d ){
	float t=a;if( b<t ) t=b;if( c<t ) t=c;if( d<t ) t=d;return t;
}

static float vmax( float a,float b,float c,float d ){
	float t=a;if( b>t ) t=b;if( c>t ) t=c;if( d>t ) t=d;return t;
}

static gxCanvas *tformCanvas( gxCanvas *c,float m[2][2],int x_handle,int y_handle ){

	vec2 v,v0,v1,v2,v3;
	float i[2][2];
	float dt=1.0f/(m[0][0]*m[1][1]-m[1][0]*m[0][1]);
	i[0][0]=dt*m[1][1];i[1][0]=-dt*m[1][0];
	i[0][1]=-dt*m[0][1];i[1][1]=dt*m[0][0];

	float ox=(float)x_handle,oy=(float)y_handle;
	v0.x=-ox;v0.y=-oy;	//tl
	v1.x=c->getWidth()-ox;v1.y=-oy;	//tr
	v2.x=c->getWidth()-ox;v2.y=c->getHeight()-oy;	//br
	v3.x=-ox;v3.y=c->getHeight()-oy;	//bl
	v0=vrot(m,v0);v1=vrot(m,v1);v2=vrot(m,v2);v3=vrot(m,v3);
	float minx=floor( vmin( v0.x,v1.x,v2.x,v3.x ) );
	float miny=floor( vmin( v0.y,v1.y,v2.y,v3.y ) );
	float maxx=ceil( vmax( v0.x,v1.x,v2.x,v3.x ) );
	float maxy=ceil( vmax( v0.y,v1.y,v2.y,v3.y ) );
	int iw=(int)(maxx-minx),ih=(int)(maxy-miny);

	gxCanvas *t=gx_graphics->createCanvas( iw,ih,0 );
	t->setHandle( (int)-minx,(int)-miny );
	t->setMask( c->getMask() );

	v.y=miny+.5f;
	for( int y=0;y<ih;++v.y,++y ){
		v.x=minx+.5f;
		for( int x=0;x<iw;++v.x,++x ){
			vec2 q=vrot(i,v);
			unsigned rgb=filter ? getPixel( c,q.x+ox,q.y+oy ) : c->getPixel((int) floor(q.x+ox),(int)floor(q.y+oy) );
			t->setPixel( x,y,rgb );
		}
	}

	return t;
}

static bool saveCanvas( gxCanvas *c,const string &f ){

	ofstream out( f.c_str(),ios::binary );
	if( !out.good() ) return false;

	int tempsize=(c->getWidth()*3+3)&~3;

	BITMAPFILEHEADER bf;
	memset( &bf,0,sizeof(bf) );
	bf.bfType='MB';
	bf.bfSize=sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER)+tempsize*c->getHeight();
	bf.bfOffBits=sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER);
	BITMAPINFOHEADER bi;memset( &bi,0,sizeof(bi) );
	bi.biSize=sizeof(bi);
	bi.biWidth=c->getWidth();
	bi.biHeight=c->getHeight();
	bi.biPlanes=1;
	bi.biBitCount=24;
	out.write( (char*)&bf,sizeof(bf) );
	out.write( (char*)&bi,sizeof(bi) );

	unsigned char *temp=d_new unsigned char[ tempsize ];
	memset( temp,0,tempsize );

	for( int y=c->getHeight()-1;y>=0;--y ){
		unsigned char *dest=temp;
		for( int x=0;x<c->getWidth();++x ){
			unsigned rgb=c->getPixel( x,y );
			*dest++=rgb&0xff;
			*dest++=(rgb>>8)&0xff;
			*dest++=(rgb>>16)&0xff;
		}
		out.write( (char*)temp,tempsize );
	}
	
	delete [] temp;

	return out.good();
}

int  bbCountGfxModes3D(){
	return 0; //TODO: find out how to reimplement this
}

int  bbCountGfxModes(){
	return bbCountGfxModes3D();
}

int  bbGfxModeWidth( int n ){
	if (!debugMode( n,"GfxModeWidth" )) return 0;
	return gfx_modes[n-1].w;
}

int  bbGfxModeHeight( int n ){
	if (!debugMode( n,"GfxModeHeight" )) return 0;
	return gfx_modes[n-1].h;
}

int  bbGfxModeDepth( int n ){
	if (!debugMode( n,"GfxModeDepth" )) return 0;
	return gfx_modes[n-1].d;
}

static int modeExists( int w,int h,int d ){
	return 0; //TODO: find out how to reimplement this
}

int  bbGfxModeExists( int w,int h,int d ){
	return modeExists( w,h,d );
}

#ifdef PRO
int  bbGfxDriver3D( int n ){
	return 1;
}

int  bbGfxMode3DExists( int w,int h,int d ){
	return modeExists( w,h,d );
}

int  bbGfxMode3D( int n ){
	return 1;
}

int  bbWindowed3D(){
	return 1;
}
#endif

int  bbTotalVidMem(){
	return -1;//TODO: check if this is necessary //gx_graphics->getTotalVidmem();
}

int  bbAvailVidMem(){
	return -1;//TODO: check if this is necessary //gx_graphics->getAvailVidmem();
}

void bbSetBuffer( gxCanvas *buff ){
	if (!debugCanvas( buff,"SetBuffer" )) return;
	gx_canvas=buff;
	curs_x=curs_y=0;
	gx_canvas->setOrigin( 0,0 );
	gx_canvas->setViewport( 0,0,gx_canvas->getWidth(),gx_canvas->getHeight() );
	gx_canvas->setColor( curr_color );
	gx_canvas->setClsColor( curr_clsColor );
	gx_canvas->setFont( curr_font );
}

gxCanvas *bbGraphicsBuffer(){
	return gx_canvas;
}

int bbLoadBuffer( gxCanvas *c,BBStr *str ){
	if (!debugCanvas( c,"LoadBuffer" )) return 0;
	string s=*str;delete str;
	gxCanvas *t=gx_graphics->loadCanvas( s,0 );
	if( !t ) return 0;
	float m[2][2];
	m[0][0]=(float)c->getWidth()/(float)t->getWidth();
	m[1][1]=(float)c->getHeight()/(float)t->getHeight();
	m[1][0]=m[0][1]=0;
	gxCanvas *p=tformCanvas( t,m,0,0 );
	gx_graphics->freeCanvas( t );
	int ox,oy;
	c->getOrigin( &ox,&oy );c->setOrigin( 0,0 );
	c->blit( 0,0,p,0,0,p->getWidth(),p->getHeight(),true );
	gx_graphics->freeCanvas( p );
	return 1;
}

int bbSaveBuffer( gxCanvas *c,BBStr *str ){
	if (!debugCanvas( c,"SaveBuffer" )) return 0;
	string t=*str;delete str;
	return saveCanvas( c,t ) ? 1 : 0;
}

/*void bbBufferDirty( gxCanvas *c ){
	if (!debugCanvas( c,"BufferDirty" )) return;
	c->backup();
}*/

static void graphics( int w,int h,int d,int flags ){
	freeGraphics();
	gx_runtime->closeGraphics( gx_graphics );
	gx_graphics=gx_runtime->openGraphics( w,h,d,flags );
	if( !gx_runtime->idle() ) RTEX( 0 );
	if( !gx_graphics ){
		RTEX( "Unable to set graphics mode" );
	}
	curr_clsColor=0;
	curr_color=0xffffffff;
	curr_font=gx_graphics->getDefaultFont();
	gxCanvas *buff=gx_graphics->getBackCanvas();
	bbSetBuffer( buff );
}

#ifdef PRO
void bbGraphics3D( int w,int h,int d,int mode ){
	int flags=gxGraphics::GRAPHICS_3D;
	switch( mode ){
	case 0:flags|=(debug && bbWindowed3D()) ? gxGraphics::GRAPHICS_WINDOWED : 0 ;break;
	case 1:break;
	case 2:flags|=gxGraphics::GRAPHICS_WINDOWED;break;
	case 3:flags|=gxGraphics::GRAPHICS_WINDOWED|gxGraphics::GRAPHICS_SCALED;break;
	case 6:flags|=gxGraphics::GRAPHICS_WINDOWED|gxGraphics::GRAPHICS_AUTOSUSPEND;break;
	case 7:flags|=gxGraphics::GRAPHICS_WINDOWED|gxGraphics::GRAPHICS_SCALED|gxGraphics::GRAPHICS_AUTOSUSPEND;break;
	default:RTEX( "Illegal Graphics3D mode" );
	}
	graphics( w,h,d,flags );
	extern void blitz3d_open();
	blitz3d_open();
}
#endif

void bbEndGraphics(){
	freeGraphics();
	gx_runtime->closeGraphics( gx_graphics );
	gx_graphics=gx_runtime->openGraphics( 400,300,0,gxGraphics::GRAPHICS_WINDOWED );
	if( !gx_runtime->idle() ) RTEX( 0 );
	if( gx_graphics ){
		curr_clsColor=0;
		curr_color=0xffffffff;
		curr_font=gx_graphics->getDefaultFont();
		bbSetBuffer( gx_graphics->getFrontCanvas() );
		return;
	}
	RTEX( "Unable to set graphics mode" );
}

int bbGraphicsLost(){
	return 0; //TODO: remove probably?
}

void bbSetGamma( int r,int g,int b,float dr,float dg,float db ){
	if( dr<0 ) dr=0;
	else if( dr>255.0f ) dr=255.0f;
	if( dg<0 ) dg=0;
	else if( dg>255.0f ) dg=255.0f;
	if( db<0 ) db=0;
	else if( db>255.0f ) db=255.0f;
	RTEX("Gamma not implemented");
	//gx_graphics->setGamma( r,g,b,dr,dg,db );
}

void  bbUpdateGamma( int calibrate ){
	RTEX("Gamma not implemented");
	//gx_graphics->updateGamma( !!calibrate );
}

float  bbGammaRed( int n ){
	float dr,dg,db;
	RTEX("Gamma not implemented");
	//gx_graphics->getGamma( n,n,n,&dr,&dg,&db );
	return dr;
}

float  bbGammaGreen( int n ){
	float dr,dg,db;
	RTEX("Gamma not implemented");
	//gx_graphics->getGamma( n,n,n,&dr,&dg,&db );
	return dg;
}

float  bbGammaBlue( int n ){
	float dr,dg,db;
	RTEX("Gamma not implemented");
	//gx_graphics->getGamma( n,n,n,&dr,&dg,&db );
	return db;
}

gxCanvas *bbFrontBuffer(){
	return gx_graphics->getFrontCanvas();
}

gxCanvas *bbBackBuffer(){
	return gx_graphics->getBackCanvas();
}

/*void bbLockBuffer( gxCanvas *buff ){
	if( buff ) { if (!debugCanvas( buff,"LockBuffer" )) return; }
	(buff ? buff : gx_canvas)->lock();
}

void bbUnlockBuffer( gxCanvas *buff ){
	if( buff ) { if (!debugCanvas( buff,"UnlockBuffer" )) return; }
	(buff ? buff : gx_canvas)->unlock();
}*/

int bbReadPixel( int x,int y,gxCanvas *buff ){
	if( buff ) { if (!debugCanvas( buff,"ReadPixel" )) return 0; }
	return (buff ? buff : gx_canvas)->getPixel( x,y );
}

void bbWritePixel( int x,int y,int argb,gxCanvas *buff ){
	if( buff ) { if (!debugCanvas( buff,"WritePixel" )) return; }
	(buff ? buff : gx_canvas)->setPixel( x,y,argb );
}

/*int bbReadPixelFast( int x,int y,gxCanvas *buff ){
	return (buff ? buff : gx_canvas)->getPixelFast( x,y );
}

void bbWritePixelFast( int x,int y,int argb,gxCanvas *buff ){
	(buff ? buff : gx_canvas)->setPixelFast( x,y,argb );
}*/

void bbCopyPixel( int src_x,int src_y,gxCanvas *src,int dest_x,int dest_y,gxCanvas *buff ){
	(buff ? buff : gx_canvas)->copyPixel( dest_x,dest_y,src ? src : gx_canvas,src_x,src_y );
}

/*void bbCopyPixelFast( int src_x,int src_y,gxCanvas *src,int dest_x,int dest_y,gxCanvas *buff ){
	(buff ? buff : gx_canvas)->copyPixelFast( dest_x,dest_y,src ? src : gx_canvas,src_x,src_y );
}

int bbScanLine(){
	return gx_graphics->getScanLine();
}

void bbVWait( int n ){
	gx_graphics->vwait();
	if( !gx_runtime->idle() ) RTEX( 0 );
}*/

void bbFlip( int vwait ){
	gx_graphics->flip( vwait ? true : false );
	if( !gx_runtime->idle() ) RTEX( 0 );
}

int bbGraphicsWidth(){
	return gx_graphics->getWidth();
}

int bbGraphicsHeight(){
	return gx_graphics->getHeight();
}

int bbGraphicsDepth(){
	return 32;
}

void bbOrigin( int x,int y ){
	gx_canvas->setOrigin( x,y );
}

void bbViewport( int x,int y,int w,int h ){
	gx_canvas->setViewport( x,y,w,h );
}

void bbColor( int r,int g,int b ){
	gx_canvas->setColor( curr_color=(r<<16)|(g<<8)|b );
}

void bbGetColor( int x,int y ){
	gx_canvas->setColor( curr_color=gx_canvas->getPixel( x,y ) );
}

int bbColorRed(){
	return (gx_canvas->getColor()>>16)&0xff;
}

int bbColorGreen(){
	return (gx_canvas->getColor()>>8)&0xff;
}

int bbColorBlue(){
	return gx_canvas->getColor()&0xff;
}

void bbClsColor( int r,int g,int b ){
	gx_canvas->setClsColor( curr_clsColor=(r<<16)|(g<<8)|b );
}

void bbSetFont( gxFont *f ){
	if (!debugFont( f,"SetFont" )) return;
	gx_canvas->setFont( curr_font=f );
}

void bbCls(){
	gx_canvas->cls();
}

void bbPlot( int x,int y ){
	gx_canvas->plot( x,y );
}

void bbLine( int x1,int y1,int x2,int y2 ){
	gx_canvas->line( x1,y1,x2,y2 );
}

void bbRect( int x,int y,int w,int h,int solid ){
	gx_canvas->rect( x,y,w,h,solid ? true : false );
}

void bbOval( int x,int y,int w,int h,int solid ){
	gx_canvas->oval( x,y,w,h,solid ? true : false );
}

void bbText( int x,int y,BBStr *str,int centre_x,int centre_y ){
	if( centre_x ) x-=curr_font->getWidth( *str )/2;
	if( centre_y ) y-=curr_font->getHeight()/2;
	gx_canvas->text( x,y,*str );
	delete str;
}

void bbCopyRect( int sx,int sy,int w,int h,int dx,int dy,gxCanvas *src,gxCanvas *dest ){
	if( src ) { if (!debugCanvas( src,"CopyRect (src)" )) return; }
	else src=gx_canvas;
	if( dest ) { if (!debugCanvas( dest,"CopyRect (dest)" )) return; }
	else dest=gx_canvas;
	dest->blit( dx,dy,src,sx,sy,w,h,true );
}

gxFont *bbLoadFont( BBStr *name,int height,int bold,int italic,int underline ){
	int flags=
		(bold ? gxFont::FONT_BOLD : 0 ) |
		(italic ? gxFont::FONT_ITALIC : 0 ) |
		(underline ? gxFont::FONT_UNDERLINE : 0 );
	gxFont *font=gx_graphics->loadFont( *name,height,flags );
	delete name;
	return font;
}

void bbFreeFont( gxFont *f ){
	if (!debugFont( f,"FreeFont" )) return;
	if( f==curr_font ) bbSetFont( gx_graphics->getDefaultFont() );
	gx_graphics->freeFont( f );
}

int bbFontWidth(){
	return curr_font->getWidth();
}

int bbFontHeight(){
	return curr_font->getHeight();
}

int bbStringWidth( BBStr *str ){
	string t=*str;delete str;
	return curr_font->getWidth( t );
}

int bbStringHeight( BBStr *str ){
	delete str;
	return curr_font->getHeight();
}

gxMovie *bbOpenMovie( BBStr *s ){
	gxMovie *movie=gx_graphics->openMovie( *s,0 );delete s;
	return movie;
}

int bbDrawMovie( gxMovie *movie,int x,int y,int w,int h ){
	if( w<0 ) w=movie->getWidth();
	if( h<0 ) h=movie->getHeight();
	int playing=movie->draw( gx_canvas,x,y,w,h );
	if( !gx_runtime->idle() ) RTEX( 0 );
	return playing;
}

int bbMovieWidth( gxMovie *movie ){
	return movie->getWidth();
}

int bbMovieHeight( gxMovie *movie ){
	return movie->getHeight();
}

int bbMoviePlaying( gxMovie *movie ){
	return movie->isPlaying();
}

void bbCloseMovie( gxMovie *movie ){
	gx_graphics->closeMovie( movie );
}

bbImage *bbLoadImage( BBStr *s ){
	string t=*s;delete s;
	gxCanvas *c=gx_graphics->loadCanvas( t,0 );
	if( !c ) return 0;
	//if( auto_dirty ) c->backup();
	if( auto_midhandle ) c->setHandle( c->getWidth()/2,c->getHeight()/2 );
	vector<gxCanvas*> frames;
	frames.push_back( c );
	bbImage *i=d_new bbImage( frames );
	image_set.insert( i );
	return i;
}

bbImage *bbLoadAnimImage( BBStr *s,int w,int h,int first,int cnt ){

	string t=*s;delete s;

	if( cnt<1 ) RTEX( "Illegal frame count" );
	if( first<0 ) RTEX( "Illegal first frame" );

	gxCanvas *pic=gx_graphics->loadCanvas( t,gxCanvas::CANVAS_NONDISPLAY );
	if( !pic ) return 0;

	//frames per row, per picture
	int fpr=pic->getWidth()/w;
	int fpp=pic->getHeight()/h*fpr;
	if( first+cnt>fpp ){
		gx_graphics->freeCanvas( pic );
		RTEX( "Not enough frames in bitmap" );
	}

	//x,y of first frame...
	vector<gxCanvas*> frames;
	int src_x=first%fpr*w,src_y=first/fpr*h;

	for( int k=0;k<cnt;++k ){
		gxCanvas *c=gx_graphics->createCanvas( w,h,0 );
		if( !c ){
			for( --k;k>=0;--k ) gx_graphics->freeCanvas( frames[k] );
			gx_graphics->freeCanvas( pic );return 0;
		}
		c->blit( 0,0,pic,src_x,src_y,w,h,true );
		//if( auto_dirty ) c->backup();
		if( auto_midhandle ) c->setHandle( c->getWidth()/2,c->getHeight()/2 );
		frames.push_back( c );
		src_x+=w;if( src_x+w>pic->getWidth() ){ src_x=0;src_y+=h; }
	}
	gx_graphics->freeCanvas( pic );
	bbImage *i=d_new bbImage( frames );
	image_set.insert( i );
	return i;
}

bbImage *bbCopyImage( bbImage *i ){
	if (!debugImage( i,0,"CopyImage" )) return 0;
	vector<gxCanvas*> frames;
	const vector<gxCanvas*> &f=i->getFrames();
	for( int k=0;k<(int)f.size();++k ){
		gxCanvas *t=f[k];
		gxCanvas *c=gx_graphics->createCanvas( t->getWidth(),t->getHeight(),0 );
		if( !c ){
			for( --k;k>=0;--k ) gx_graphics->freeCanvas( frames[k] );
			return 0;
		}
		int x,y;
		t->getHandle( &x,&y );
		t->setHandle( 0,0 );
		c->blit( 0,0,t,0,0,t->getWidth(),t->getHeight(),true );
		//if( auto_dirty ) c->backup();
		t->setHandle( x,y );
		c->setHandle( x,y );
		c->setMask( t->getMask() );
		frames.push_back( c );
	}
	bbImage *t=d_new bbImage( frames );
	image_set.insert( t );
	return t;
}

bbImage *bbCreateImage( int w,int h,int n ){
	vector<gxCanvas*> frames;
	for( int k=0;k<n;++k ){
		gxCanvas *c=gx_graphics->createCanvas( w,h,0 );
		if( !c ){
			for( --k;k>=0;--k ) gx_graphics->freeCanvas( frames[k] );
			return 0;
		}
		//if( auto_dirty ) c->backup();
		if( auto_midhandle ) c->setHandle( c->getWidth()/2,c->getHeight()/2 );
		frames.push_back( c );
	}
	bbImage *i=d_new bbImage( frames );
	image_set.insert( i );
	return i;
}

void bbFreeImage( bbImage *i ){
	if( !image_set.erase(i) ) return;
	const vector<gxCanvas*> &f=i->getFrames();
	for( int k=0;k<(int)f.size();++k ){
		if( f[k]==gx_canvas ){
			bbSetBuffer( gx_graphics->getFrontCanvas() );
			break;
		}
	}
	delete i;
}

int bbSaveImage( bbImage *i,BBStr *str,int n ){
	if (!debugImage( i,n,"SaveImage" )) return 0;
	string t=*str;delete str;
	gxCanvas *c=i->getFrames()[n];
	return saveCanvas( c,t ) ? 1 : 0;
}

void bbGrabImage( bbImage *i,int x,int y,int n ){
	if (!debugImage( i,n,"GrabImage" )) return;
	gxCanvas *c=i->getFrames()[n];
	int src_ox,src_oy,dst_hx,dst_hy;
	gx_canvas->getOrigin( &src_ox,&src_oy );
	c->getHandle( &dst_hx,&dst_hy );
	x+=src_ox-dst_hx;y+=src_oy-dst_hy;
	c->setViewport( 0,0,c->getWidth(),c->getHeight() );
	c->blit( 0,0,gx_canvas,x,y,c->getWidth(),c->getHeight(),true );
	//if( auto_dirty ) c->backup();
}

gxCanvas *bbImageBuffer( bbImage *i,int n ){
	if (!debugImage( i,n,"ImageBuffer" )) return 0;
	return i->getFrames()[n];
}

void bbDrawImage( bbImage *i,int x,int y,int frame ){
	if (!debugImage( i,frame,"DrawImage" )) return;
	gxCanvas *c=i->getFrames()[frame];
	gx_canvas->blit( x,y,c,0,0,c->getWidth(),c->getHeight(),false );
}

void bbDrawBlock( bbImage *i,int x,int y,int frame ){
	if (!debugImage( i,frame,"DrawBlock" )) return;
	gxCanvas *c=i->getFrames()[frame];
	gx_canvas->blit( x,y,c,0,0,c->getWidth(),c->getHeight(),true );
}

static void tile( bbImage *i,int x,int y,int frame,bool solid ){
	gxCanvas *c=i->getFrames()[frame];

	int hx,hy;
	c->getHandle( &hx,&hy );
	int w=c->getWidth(),h=c->getHeight();

	int ox,oy,vp_x,vp_y,vp_w,vp_h;
	gx_canvas->getOrigin( &ox,&oy );
	gx_canvas->getViewport( &vp_x,&vp_y,&vp_w,&vp_h );
	int dx=vp_x-ox+hx;
	int dy=vp_y-oy+hy;
	x-=dx;
	y-=dy;
	dx+=(x>=0?x%w:w-(-x%w));
	dy+=(y>=0?y%h:h-(-y%h));

	for( y=-h;y<vp_h;y+=h ){
		for( x=-w;x<vp_w;x+=w ){
			gx_canvas->blit( x+dx,y+dy,c,0,0,w,h,solid );
		}
	}
}

void bbTileImage( bbImage *i,int x,int y,int frame ){
	if (!debugImage( i,frame,"TileImage" )) return;
	tile( i,x,y,frame,false );
}

void bbTileBlock( bbImage *i,int x,int y,int frame ){
	if (!debugImage( i,frame,"TileBlock" )) return;
	tile( i,x,y,frame,true );
}

void bbDrawImageRect( bbImage *i,int x,int y,int r_x,int r_y,int r_w,int r_h,int frame ){
	if (!debugImage( i,frame,"DrawImageRect" )) return;
	gxCanvas *c=i->getFrames()[frame];
	gx_canvas->blit( x,y,c,r_x,r_y,r_w,r_h,false );
}

void bbDrawBlockRect( bbImage *i,int x,int y,int r_x,int r_y,int r_w,int r_h,int frame ){
	if (!debugImage( i,frame,"DrawBlockRect" )) return;
	gxCanvas *c=i->getFrames()[frame];
	gx_canvas->blit( x,y,c,r_x,r_y,r_w,r_h,true );
}

void bbMaskImage( bbImage *i,int r,int g,int b ){
	if (!debugImage( i,0,"MaskImage" )) return;
	unsigned argb=(r<<16)|(g<<8)|b;
	const vector<gxCanvas*> &f=i->getFrames();
	for( int k=0;k<(int)f.size();++k ) f[k]->setMask( argb );
}

void bbHandleImage( bbImage *i,int x,int y ){
	if (!debugImage( i,0,"HandleImage" )) return;
	const vector<gxCanvas*> &f=i->getFrames();
	for( int k=0;k<(int)f.size();++k ) f[k]->setHandle( x,y );
}

void bbMidHandle( bbImage *i ){
	if (!debugImage( i,0,"MidHandle" )) return;
	const vector<gxCanvas*> &f=i->getFrames();
	for( int k=0;k<(int)f.size();++k ) f[k]->setHandle( f[k]->getWidth()/2,f[k]->getHeight()/2 );
}

void bbAutoMidHandle( int enable ){
	auto_midhandle=enable ? true : false;
}

int bbImageWidth( bbImage *i ){
	if (!debugImage( i,0,"ImageWidth" )) return 0;
	return i->getFrames()[0]->getWidth();
}

int bbImageHeight( bbImage *i ){
	if (!debugImage( i,0,"ImageHeight" )) return 0;
	return i->getFrames()[0]->getHeight();
}

int bbImageXHandle( bbImage *i ){
	if (!debugImage( i,0,"ImageXHandle" )) return 0;
	int x,y;
	i->getFrames()[0]->getHandle( &x,&y );
	return x;
}

int bbImageYHandle( bbImage *i ){
	if (!debugImage( i,0,"ImageYHandle" )) return 0;
	int x,y;
	i->getFrames()[0]->getHandle( &x,&y );
	return y;
}

int bbImagesOverlap( bbImage *i1,int x1,int y1,bbImage *i2,int x2,int y2 ){
	if (!debugImage( i1,0,"ImagesOverlap (i1)" )) return 0;
	if (!debugImage( i2,0,"ImagesOverlap (i2)" )) return 0;
	gxCanvas *c1=i1->getFrames()[0];
	gxCanvas *c2=i2->getFrames()[0];
	return c1->collide( x1,y1,c2,x2,y2,true );
}

int bbImagesCollide( bbImage *i1,int x1,int y1,int f1,bbImage *i2,int x2,int y2,int f2 ){
	if (!debugImage( i1,f1,"ImagesCollide (i1)" )) return 0;
	if (!debugImage( i2,f2,"ImagesCollide (i2)" )) return 0;
	gxCanvas *c1=i1->getFrames()[f1];
	gxCanvas *c2=i2->getFrames()[f2];
	return c1->collide( x1,y1,c2,x2,y2,false );
}

int bbRectsOverlap( int x1,int y1,int w1,int h1,int x2,int y2,int w2,int h2 ){
	if( x1+w1<=x2 || x1>=x2+w2 || y1+h1<=y2 || y1>=y2+h2 ) return 0;
	return 1;
}

int bbImageRectOverlap( bbImage *i,int x,int y,int x2,int y2,int w2,int h2 ){
	if (!debugImage( i,0,"ImageRectOverlap" )) return 0;
	gxCanvas *c=i->getFrames()[0];
	return c->rect_collide( x,y,x2,y2,w2,h2,true );
}

int bbImageRectCollide( bbImage *i,int x,int y,int f,int x2,int y2,int w2,int h2 ){
	if (!debugImage( i,f,"ImageRectCollide" )) return 0;
	gxCanvas *c=i->getFrames()[f];
	return c->rect_collide( x,y,x2,y2,w2,h2,false );
}

void bbTFormImage( bbImage *i,float a,float b,float c,float d ){
	if (!debugImage( i,0,"TFormImage" )) return;
	const vector<gxCanvas*> &f=i->getFrames();
	int k;
	for( k=0;k<(int)f.size();++k ){
		if( f[k]==gx_canvas ){
			bbSetBuffer( gx_graphics->getFrontCanvas() );
			break;
		}
	}
	float m[2][2];
	m[0][0]=a;m[1][0]=b;m[0][1]=c;m[1][1]=d;
	for( k=0;k<(int)f.size();++k ){
		gxCanvas *c=f[k];
		int hx,hy;c->getHandle( &hx,&hy );
		gxCanvas *t=tformCanvas( c,m,hx,hy );
		i->replaceFrame( k,t );
		//t->backup();
	}
}

void bbScaleImage( bbImage *i,float w,float h ){
	if (!debugImage( i,0,"ScaleImage" )) return;
	bbTFormImage( i,w,0,0,h );
}

void bbResizeImage( bbImage *i,float w,float h ){
	if (!debugImage( i,0,"ResizeImage" )) return;
	gxCanvas *c=i->getFrames()[0];
	bbTFormImage( i,w/(float)c->getWidth(),0,0,h/(float)c->getHeight() );
}

void bbRotateImage( bbImage *i,float d ){
	if (!debugImage( i,0,"RotateImage" )) return;
	d*=-dtor;
	bbTFormImage( i,cos(d),-sin(d),sin(d),cos(d) );
}

void bbTFormFilter( int enable ){
	filter=enable ? true : false;
}

static int p_ox,p_oy,p_hx,p_hy,p_vpx,p_vpy,p_vpw,p_vph;

static gxCanvas *startPrinting(){
	
	gxCanvas *c=gx_graphics->getFrontCanvas();

	//c->lock();
	//c->unlock();

	c->getOrigin( &p_ox,&p_oy );
	c->getHandle( &p_hx,&p_hy );
	c->getViewport( &p_vpx,&p_vpy,&p_vpw,&p_vph );

	c->setOrigin( 0,0 );
	c->setHandle( 0,0 );
	c->setViewport( 0,0,c->getWidth(),c->getHeight() );
	if( c!=gx_canvas ){
		c->setFont( curr_font );
		c->setColor( curr_color );
	}

	int dy=curs_y+curr_font->getHeight()-c->getHeight();
	if( dy>0 ){
		curs_y=c->getHeight()-curr_font->getHeight();
		c->blit( 0,0,c,0,dy,c->getWidth(),c->getHeight()-dy,true );
		c->setColor( curr_clsColor );
		c->rect( 0,c->getHeight()-dy,c->getWidth(),dy,true );
		c->setColor( curr_color );
	}
	return c;
}

static void endPrinting( gxCanvas *c ){
	c->setViewport( p_vpx,p_vpy,p_vpw,p_vph );
	c->setHandle( p_hx,p_hy );
	c->setOrigin( p_ox,p_oy );
	if( c==gx_canvas ) c->setColor( curr_color );
	if( !gx_runtime->idle() ) RTEX( 0 );
}

void bbWrite( BBStr *str ){
	gxCanvas *c=startPrinting();
	c->text( curs_x,curs_y,*str );
	curs_x+=curr_font->getWidth( *str );
	endPrinting( c );
	delete str;
}

void bbPrint( BBStr *str ){
	gxCanvas *c=startPrinting();
	c->text( curs_x,curs_y,*str );
	curs_x=0;
	curs_y+=curr_font->getHeight();
	endPrinting( c );
	delete str;
}

BBStr *bbInput( BBStr *prompt ){
	gxCanvas *c=startPrinting();
	string t=*prompt;delete prompt;

	//get temp canvas
	if( !p_canvas || p_canvas->getWidth()<c->getWidth() || p_canvas->getHeight()<curr_font->getHeight()*2 ){
		if( p_canvas ) gx_graphics->freeCanvas( p_canvas );
		p_canvas=gx_graphics->createCanvas( c->getWidth(),curr_font->getHeight()*2,0 );
		if( !p_canvas ){
			endPrinting(c);
			return d_new BBStr();
		}
	}
	//draw prompt
	c->text( curs_x,curs_y,t );
	curs_x+=curr_font->getWidth( t );

	p_canvas->setFont( curr_font );
	p_canvas->setColor( curr_color );
	p_canvas->blit( 0,0,c,0,curs_y,c->getWidth(),curr_font->getHeight(),true );

	string str;
	bool go=true;
	int curs=0,last_key=0,last_time,rep_delay;

	while( go ){

		//render all text
		//calc curs x and width
		int cx=curs_x+curr_font->getWidth( str.substr( 0,curs ) );
		int cw=curr_font->getWidth( curs<(int)str.size() ? str.substr( curs,1 ) : "X" );

		//wait for a key
		int key=0,st=gx_runtime->getMilliSecs(),tc=-1;

		while( gx_runtime->idle() ){
			int t=gx_runtime->getMilliSecs();
			int n=(t-st)/320;
			if( n!=tc ){
				tc=n;
				if( !(tc&1) ){	//cursor ON
					c->setColor( curr_clsColor^0xffffff );
					c->rect( cx,curs_y,cw,curr_font->getHeight(),true );
					c->setColor( curr_clsColor );
				}else{			//cursor OFF
					c->blit( cx,curs_y,p_canvas,cx,0,cw,curr_font->getHeight(),true );
					c->setColor( curr_color );
				}
				c->text( cx,curs_y,str.substr( curs,1 ) );
			}
			if( key=gx_keyboard->getKey() ){
				if( int asc=gx_input->toAscii( key ) ){
					rep_delay=280;
					last_key=key;
					last_time=t;
					key=asc;
					break;
				}
			}
			if( last_key && gx_keyboard->keyDown( last_key ) ){
				if( t-last_time>rep_delay ){
					if( key=gx_input->toAscii( last_key ) ){
						last_time+=rep_delay;
						rep_delay=40;
						break;
					}
				}
			}else last_key=0;
			gx_runtime->delay( 20 );
		}

		//check the key
		switch( key ){
		case 0:
			go=false;
			str="";
			break;
		case 8:
			if( curs ){
				str=str.substr( 0,curs-1 )+str.substr( curs );
				--curs;
			}
			break;
		case 27:
			curs=0;str="";
			break;
		case gxInput::ASC_DELETE:
			if( curs<(int)str.size() ) str=str.substr( 0,curs )+str.substr( curs+1 );
			break;
		case gxInput::ASC_HOME:
			curs=0;
			break;
		case gxInput::ASC_END:
			curs=str.size();
			break;
		case gxInput::ASC_LEFT:
			if( curs ) --curs;
			break;
		case gxInput::ASC_RIGHT:
			if( curs<(int)str.size() ) ++curs;
			break;
		case '\r':
			go=false;
			break;
		default:
			if( curr_font->isPrintable( key ) ){
				str=str.substr(0,curs)+char(key)+str.substr(curs);
				++curs;
			}
		}

		//render text
		p_canvas->blit( 0,curr_font->getHeight(),p_canvas,0,0,c->getWidth(),curr_font->getHeight(),true );
		p_canvas->text( curs_x,curr_font->getHeight(),str );
		c->blit( 0,curs_y,p_canvas,0,curr_font->getHeight(),c->getWidth(),curr_font->getHeight(),true );
	}

	curs_x=0;
	curs_y+=curr_font->getHeight();
	endPrinting( c );
	return d_new BBStr( str );
}

void bbLocate( int x,int y ){
	gxCanvas *c=gx_graphics->getFrontCanvas();
	curs_x=x<0 ? 0 : (x > c->getWidth() ? c->getWidth() : x);
	curs_y=y<0 ? 0 : (y > c->getHeight() ? c->getHeight() : y);
}

void bbShowPointer(){
	gx_runtime->setPointerVisible( true );
}

void bbHidePointer(){
	gx_runtime->setPointerVisible( false );
}

bool graphics_create(){
	p_canvas=0;
	filter=true;
	freeGraphics();
	auto_dirty=true;
	auto_midhandle=false;
	gx_graphics=gx_runtime->openGraphics( 400,300,0,gxGraphics::GRAPHICS_WINDOWED );
	if( gx_graphics ){
		curr_clsColor=0;
		curr_color=0xffffffff;
		curr_font=gx_graphics->getDefaultFont();
		bbSetBuffer( bbFrontBuffer() );
		return true;
	}
	return false;
}

bool graphics_destroy(){
	freeGraphics();
	gfx_modes.clear();
	if( gx_graphics ){
		gx_runtime->closeGraphics( gx_graphics );
		gx_graphics=0;
	}
	return true;
}

void graphics_link( void (*rtSym)( const char *sym,void *pc ) ){

	//gfx driver info
	//rtSym( "%CountGfxDrivers",bbCountGfxDrivers );
	//rtSym( "$GfxDriverName%driver",bbGfxDriverName );
	//rtSym( "SetGfxDriver%driver",bbSetGfxDriver );

	//gfx mode info
	rtSym( "%CountGfxModes",bbCountGfxModes );
	rtSym( "%GfxModeExists%width%height%depth",bbGfxModeExists );

	rtSym( "%GfxModeWidth%mode",bbGfxModeWidth );
	rtSym( "%GfxModeHeight%mode",bbGfxModeHeight );
	rtSym( "%GfxModeDepth%mode",bbGfxModeDepth );
	rtSym( "%AvailVidMem",bbAvailVidMem );
	rtSym( "%TotalVidMem",bbTotalVidMem );

#ifdef PRO
	rtSym( "%GfxDriver3D%driver",bbGfxDriver3D );
	rtSym( "%CountGfxModes3D",bbCountGfxModes3D );
	rtSym( "%GfxMode3DExists%width%height%depth",bbGfxMode3DExists );
	rtSym( "%GfxMode3D%mode",bbGfxMode3D );
	rtSym( "%Windowed3D",bbWindowed3D );
#endif

	//display mode
	rtSym( "Graphics%width%height%depth=0%mode=0",bbGraphics3D ); //TODO: consider removing Graphics symbol altogether
#ifdef PRO
	rtSym( "Graphics3D%width%height%depth=0%mode=0",bbGraphics3D );
#endif
	rtSym( "EndGraphics",bbEndGraphics );
	rtSym( "%GraphicsLost",bbGraphicsLost );

	rtSym( "SetGamma%src_red%src_green%src_blue#dest_red#dest_green#dest_blue",bbSetGamma );
	rtSym( "UpdateGamma%calibrate=0",bbUpdateGamma );
	rtSym( "#GammaRed%red",bbGammaRed );
	rtSym( "#GammaGreen%green",bbGammaGreen );
	rtSym( "#GammaBlue%blue",bbGammaBlue );

	rtSym( "(BBBuffer)FrontBuffer",bbFrontBuffer );
	rtSym( "(BBBuffer)BackBuffer",bbBackBuffer );
	//rtSym( "%ScanLine",bbScanLine );
	//rtSym( "VWait%frames=1",bbVWait );
	rtSym( "Flip%vwait=1",bbFlip );
	rtSym( "%GraphicsWidth",bbGraphicsWidth );
	rtSym( "%GraphicsHeight",bbGraphicsHeight );
	rtSym( "%GraphicsDepth",bbGraphicsDepth );

	//buffer management
	rtSym( "SetBuffer(BBBuffer)buffer",bbSetBuffer );
	rtSym( "(BBBuffer)GraphicsBuffer",bbGraphicsBuffer );
	rtSym( "%LoadBuffer(BBBuffer)buffer$bmpfile",bbLoadBuffer );
	rtSym( "%SaveBuffer(BBBuffer)buffer$bmpfile",bbSaveBuffer );
	//rtSym( "BufferDirty(BBBuffer)buffer",bbBufferDirty );

	//fast pixel reads/write
	//rtSym( "LockBuffer(BBBuffer)buffer=Null",bbLockBuffer );
	//rtSym( "UnlockBuffer(BBBuffer)buffer=Null",bbUnlockBuffer );
	rtSym( "%ReadPixel%x%y(BBBuffer)buffer=Null",bbReadPixel );
	rtSym( "WritePixel%x%y%argb(BBBuffer)buffer=Null",bbWritePixel );
	//rtSym( "%ReadPixelFast%x%y(BBBuffer)buffer=Null",bbReadPixelFast );
	//rtSym( "WritePixelFast%x%y%argb(BBBuffer)buffer=Null",bbWritePixelFast );
	rtSym( "CopyPixel%src_x%src_y(BBBuffer)src_buffer%dest_x%dest_y(BBBuffer)dest_buffer=Null",bbCopyPixel );
	//rtSym( "CopyPixelFast%src_x%src_y(BBBuffer)src_buffer%dest_x%dest_y(BBBuffer)dest_buffer=Null",bbCopyPixelFast );

	//rendering
	rtSym( "Origin%x%y",bbOrigin );
	rtSym( "Viewport%x%y%width%height",bbViewport );
	rtSym( "Color%red%green%blue",bbColor );
	rtSym( "GetColor%x%y",bbGetColor );
	rtSym( "%ColorRed",bbColorRed );
	rtSym( "%ColorGreen",bbColorGreen );
	rtSym( "%ColorBlue",bbColorBlue );
	rtSym( "ClsColor%red%green%blue",bbClsColor );
	rtSym( "SetFont(BBFont)font",bbSetFont );
	rtSym( "Cls",bbCls );
	rtSym( "Plot%x%y",bbPlot );
	rtSym( "Rect%x%y%width%height%solid=1",bbRect );
	rtSym( "Oval%x%y%width%height%solid=1",bbOval );
	rtSym( "Line%x1%y1%x2%y2",bbLine );
	rtSym( "Text%x%y$text%centre_x=0%centre_y=0",bbText );
	rtSym( "CopyRect%source_x%source_y%width%height%dest_x%dest_y(BBBuffer)src_buffer=Null(BBBuffer)dest_buffer=Null",bbCopyRect );

	//fonts
	rtSym( "(BBFont)LoadFont$fontname%height=12%bold=0%italic=0%underline=0",bbLoadFont );
	rtSym( "FreeFont(BBFont)font",bbFreeFont );
	rtSym( "%FontWidth",bbFontWidth );
	rtSym( "%FontHeight",bbFontHeight );
	rtSym( "%StringWidth$string",bbStringWidth );
	rtSym( "%StringHeight$string",bbStringHeight );

	//movies
	rtSym( "%OpenMovie$file",bbOpenMovie );
	rtSym( "%DrawMovie%movie%x=0%y=0%w=-1%h=-1",bbDrawMovie );
	rtSym( "%MovieWidth%movie",bbMovieWidth );
	rtSym( "%MovieHeight%movie",bbMovieHeight );
	rtSym( "%MoviePlaying%movie",bbMoviePlaying );
	rtSym( "CloseMovie%movie",bbCloseMovie );

	rtSym( "(BBImage)LoadImage$bmpfile",bbLoadImage );
	rtSym( "(BBImage)LoadAnimImage$bmpfile%cellwidth%cellheight%first%count",bbLoadAnimImage );
	rtSym( "(BBImage)CopyImage(BBImage)image",bbCopyImage );
	rtSym( "(BBImage)CreateImage%width%height%frames=1",bbCreateImage );
	rtSym( "FreeImage(BBImage)image",bbFreeImage );
	rtSym( "%SaveImage(BBImage)image$bmpfile%frame=0",bbSaveImage );

	rtSym( "GrabImage(BBImage)image%x%y%frame=0",bbGrabImage );
	rtSym( "(BBBuffer)ImageBuffer(BBImage)image%frame=0",bbImageBuffer );
	rtSym( "DrawImage(BBImage)image%x%y%frame=0",bbDrawImage );
	rtSym( "DrawBlock(BBImage)image%x%y%frame=0",bbDrawBlock );
	rtSym( "TileImage(BBImage)image%x=0%y=0%frame=0",bbTileImage );
	rtSym( "TileBlock(BBImage)image%x=0%y=0%frame=0",bbTileBlock );
	rtSym( "DrawImageRect(BBImage)image%x%y%rect_x%rect_y%rect_width%rect_height%frame=0",bbDrawImageRect );
	rtSym( "DrawBlockRect(BBImage)image%x%y%rect_x%rect_y%rect_width%rect_height%frame=0",bbDrawBlockRect );
	rtSym( "MaskImage(BBImage)image%red%green%blue",bbMaskImage );
	rtSym( "HandleImage(BBImage)image%x%y",bbHandleImage );
	rtSym( "MidHandle(BBImage)image",bbMidHandle );
	rtSym( "AutoMidHandle%enable",bbAutoMidHandle );
	rtSym( "%ImageWidth(BBImage)image",bbImageWidth );
	rtSym( "%ImageHeight(BBImage)image",bbImageHeight );
	rtSym( "%ImageXHandle(BBImage)image",bbImageXHandle );
	rtSym( "%ImageYHandle(BBImage)image",bbImageYHandle );

	rtSym( "ScaleImage(BBImage)image#xscale#yscale",bbScaleImage );
	rtSym( "ResizeImage(BBImage)image#width#height",bbResizeImage );
	rtSym( "RotateImage(BBImage)image#angle",bbRotateImage );
	rtSym( "TFormImage(BBImage)image#a#b#c#d",bbTFormImage );
	rtSym( "TFormFilter%enable",bbTFormFilter );

	rtSym( "%ImagesOverlap(BBImage)image1%x1%y1%image2%x2%y2",bbImagesOverlap );
	rtSym( "%ImagesCollide(BBImage)image1%x1%y1%frame1%image2%x2%y2%frame2",bbImagesCollide );
	rtSym( "%RectsOverlap%x1%y1%width1%height1%x2%y2%width2%height2",bbRectsOverlap );
	rtSym( "%ImageRectOverlap(BBImage)image%x%y%rect_x%rect_y%rect_width%rect_height",bbImageRectOverlap );
	rtSym( "%ImageRectCollide(BBImage)image%x%y%frame%rect_x%rect_y%rect_width%rect_height",bbImageRectCollide );

	rtSym( "Write$string",bbWrite );
	rtSym( "Print$string=\"\"",bbPrint );
	rtSym( "$Input$prompt=\"\"",bbInput );
	rtSym( "Locate%x%y",bbLocate );

	rtSym( "ShowPointer",bbShowPointer );
	rtSym( "HidePointer",bbHidePointer );
}
