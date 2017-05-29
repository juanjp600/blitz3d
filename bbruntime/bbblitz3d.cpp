
#include "std.h"

#ifdef PRO

#include "bbblitz3d.h"
#include "bbgraphics.h"
#include "../blitz3d/blitz3d.h"
#include "../blitz3d/world.h"
#include "../blitz3d/texture.h"
#include "../blitz3d/brush.h"
#include "../blitz3d/camera.h"
#include "../blitz3d/sprite.h"
#include "../blitz3d/meshmodel.h"
#include "../blitz3d/loader_x.h"
#include "../blitz3d/loader_3ds.h"
#include "../blitz3d/loader_b3d.h"
#include "../blitz3d/md2model.h"
#include "../blitz3d/q3bspmodel.h"
#include "../blitz3d/meshutil.h"
#include "../blitz3d/pivot.h"
#include "../blitz3d/planemodel.h"
#include "../blitz3d/terrain.h"
#include "../blitz3d/listener.h"
#include "../blitz3d/cachedtexture.h"

gxScene *gx_scene;
extern gxFileSystem *gx_filesys;

static int tri_count;
static World *world;

static set<Brush*> brush_set;
static set<Texture*> texture_set;
static set<Entity*> entity_set;

static Listener *listener;

static bool stats_mode;

//converts 0...255 color to 0...1
static const float ctof=1.0f/255.0f;

//degrees to radians and back
static const float dtor=0.0174532925199432957692369076848861f;
static const float rtod=1/dtor;

static Vector projected,tformed;

static ObjCollision picked;

extern float stats3d[10];

static Loader_X loader_x;
static Loader_3DS loader_3ds;
static Loader_B3D loader_b3d;

static map<string,Transform> loader_mat_map;

static inline bool debug3d(const char* a){
	if( debug ) {
		if ( !gx_scene ) {
			RTEX( "3D Graphics mode not set" );
			return false;
		}
	} else {
		if ( !gx_scene ) {
			errorLog.push_back(std::string(a)+std::string(": 3D Graphics mode not set"));
			return false;
		}
	}
	return true;
}
static inline bool debugTexture( Texture *t,const char* a ){
	if( debug ) {
		if ( !texture_set.count( t ) ) {
			RTEX( "Texture does not exist" );
			return false;
		}
	} else {
		if ( !texture_set.count( t ) ) {
			errorLog.push_back(std::string(a)+std::string(": Texture does not exist"));
			return false;
		}
	}
	return true;
}
static inline bool debugBrush( Brush *b,const char* a ){
	if( debug ) {
		if ( !brush_set.count( b ) ) {
			RTEX( "Brush does not exist" );
			return false;
		}
	} else {
		if ( !brush_set.count( b ) ) {
			errorLog.push_back(std::string(a)+std::string(": Brush does not exist"));
			return false;
		}
	}
	return true;
}
static inline bool debugEntity( Entity *e,const char* a ){
	if( debug ) {
		if ( !entity_set.count(e) ) {
			RTEX( "Entity does not exist" );
			return false;
		}
	} else {
		if ( !entity_set.count(e) ) {
			errorLog.push_back(std::string(a)+std::string(": Entity does not exist"));
			return false;
		}
	}
	return true;
}
static inline bool debugParent( Entity *e,const char* a ){
	if( debug ){
		if (!debug3d("")) {
			return false;
		}
		if( e && !entity_set.count(e) ) {
			RTEX( "Parent entity does not exist" );
			return false;
		}
	} else {
		if (!debug3d(a)) {
			return false;
		}
		if( e && !entity_set.count(e) ) {
			errorLog.push_back(std::string(a)+std::string(": Parent entity does not exist"));
			return false;
		}
	}
	return true;
}
static inline bool debugMesh( MeshModel *m,const char* a ){
	if( debug ){
		if ( !debugEntity(m,"") ) {
			return false;
		}
		if( !m->getMeshModel() ) {
			RTEX( "Entity is not a mesh" );
			return false;
		}
	} else {
		if ( !debugEntity(m,a) ) {
			return false;
		}
		if( !m->getMeshModel() ) {
			errorLog.push_back(std::string(a)+std::string(": Entity is not a mesh"));
			return false;
		}
	}
	return true;
}
static inline bool debugObject( Object *o,const char* a ){
	if( debug ){
		if ( !debugEntity(o,"") ) {
			return false;
		}
		if( !o->getObject() ) {
			RTEX( "Entity is not an object" );
			return false;
		}
	} else {
		if ( !debugEntity(o,a) ) {
			return false;
		}
		if( !o->getObject() ) {
			errorLog.push_back(std::string(a)+std::string(": Entity is not an object"));
			return false;
		}
	}
	return true;
}
static inline bool debugColl( Object *o,int index,const char* a ){
	if( debug ){
		if ( !debugObject(o,"") ) {
			return false;
		}
		if( index<1 || index>o->getCollisions().size() ) {
			RTEX( "Collision index out of range" );
			return false;
		}
	} else {
		if ( !debugObject(o,a) ) {
			return false;
		}
		if( index<1 || index>o->getCollisions().size() ) {
			errorLog.push_back(std::string(a)+std::string(": Collision index out of range"));
			return false;
		}
	}
	return true;
}
static inline bool debugCamera( Camera *c,const char* a ){
	if( debug ){
		if ( !debugEntity(c,"") ) {
			return false;
		}
		if( !c->getCamera() ) {
			RTEX( "Entity is not a camera" );
			return false;
		}
	} else {
		if ( !debugEntity(c,a) ) {
			return false;
		}
		if( !c->getCamera() ) {
			errorLog.push_back(std::string(a)+std::string(": Entity is not a camera"));
			return false;
		}
	}
	return true;
}
static inline bool debugLight( Light *l,const char* a ){
	if( debug ){
		if ( !debugEntity(l,"") ) {
			return false;
		}
		if ( !l->getLight() ) {
			RTEX( "Entity is not a light" );
			return false;
		}
	} else {
		if ( !debugEntity(l,a) ) {
			return false;
		}
		if ( !l->getLight() ) {
			errorLog.push_back(std::string(a)+std::string(": Entity is not a light"));
			return false;
		}
	}
	return true;
}
static inline bool debugModel( Model *m,const char* a ){
	if( debug ){
		if ( !debugEntity(m,"") ) {
			return false;
		}
		if( !m->getModel() ) {
			RTEX( "Entity is not a model" );
			return false;
		}
	} else {
		if ( !debugEntity(m,a) ) {
			return false;
		}
		if( !m->getModel() ) {
			errorLog.push_back(std::string(a)+std::string(": Entity is not a model"));
			return false;
		}
	}
	return true;
}
static inline bool debugSprite( Sprite *s,const char* a ){
	if( debug ){
		if ( !debugModel(s,"") ) {
			return false;
		}
		if( !s->getSprite() ) {
			RTEX( "Entity is not a sprite" );
			return false;
		}
	} else {
		if ( !debugModel(s,a) ) {
			return false;
		}
		if( !s->getSprite() ) {
			errorLog.push_back(std::string(a)+std::string(": Entity is not a sprite"));
			return false;
		}
	}
	return true;
}
static inline bool debugMD2( MD2Model *m,const char* a ){
	if( debug ){
		if ( !debugModel(m,"") ) {
			return false;
		}
		if( !m->getMD2Model() ) {
			RTEX( "Entity is not an MD2 Model" );
			return false;
		}
	} else {
		if ( !debugModel(m,a) ) {
			return false;
		}
		if( !m->getMD2Model() ) {
			errorLog.push_back(std::string(a)+std::string(": Entity is not an MD2 Model"));
			return false;
		}
	}
	return true;
}
static inline bool debugBSP( Q3BSPModel *m,const char* a ){
	if( debug ){
		if ( !debugModel(m,"") ) {
			return false;	
		}
		if( !m->getBSPModel() ) {
			RTEX( "Entity is not a BSP Model" );
			return false;
		}
	} else {
		if ( !debugModel(m,a) ) {
			return false;	
		}
		if( !m->getBSPModel() ) {
			errorLog.push_back(std::string(a)+std::string(": Entity is not a BSP Model"));
			return false;
		}
	}
	return true;
}
static inline bool debugTerrain( Terrain *t,const char* a ){
	if( debug ){
		if ( !debugModel(t,"") ) {
			return false;
		}
		if( !t->getTerrain() ) {
			RTEX( "Entity is not a terrain" );
			return false;
		}
	} else {
		if ( !debugModel(t,"") ) {
			return false;
		}
		if( !t->getTerrain() ) {
			errorLog.push_back(std::string(a)+std::string(": Entity is not a terrain"));
			return false;
		}
	}
	return true;
}
static inline bool debugSegs( int n,const char* a ){
	if( debug ){
		if (!debug3d("")) {
			return false;
		}
		if( n<3 || n>50 ) {
			RTEX( "Illegal number of segments" );
			return false;
		}
	} else {
		if (!debug3d(a)) {
			return false;
		}
		if( n<3 || n>50 ) {
			errorLog.push_back(std::string(a)+std::string(": Illegal number of segments"));
			return false;
		}
	}
	return true;
}
static inline bool debugVertex( Surface *s,int n,const char* a ){
	if( debug ){
		if (!debug3d("")) {
			return false;
		}
		if( n<0 || n>=s->numVertices() ) {
			RTEX( "Vertex index out of range" );
			return false;
		}
	} else {
		if (!debug3d(a)) {
			return false;
		}
		if( n<0 || n>=s->numVertices() ) {
			errorLog.push_back(std::string(a)+std::string(": Vertex index out of range"));
			return false;
		}
	}
	return true;
}
static inline bool debugVertex( Surface *s,int n,int t,const char* a ){
	if( debug ){
		if (!debug3d("")) {
			return false;
		}
		if( n<0 || n>=s->numVertices() ) {
			RTEX( "Vertex index out of range" );
			return false;
		}
		if( t<0 || t>1 ) {
			RTEX( "Texture coordinate set out of range" );
			return false;
		}
	} else {
		if (!debug3d(a)) {
			return false;
		}
		if( n<0 || n>=s->numVertices() ) {
			errorLog.push_back(std::string(a)+std::string(": Vertex index out of range"));
			return false;
		}
		if( t<0 || t>1 ) {
			errorLog.push_back(std::string(a)+std::string(": Texture coordinate set out of range"));
			return false;
		}
	}
	return true;
}

static Entity *loadEntity( string t,int hint ){
	t=tolower(t);
	int n=t.rfind( "." );if( n==string::npos ) return 0;
	string ext=t.substr( n+1 );
	MeshLoader *l;

	if( ext=="x" ) l=&loader_x;
	else if( ext=="3ds" ) l=&loader_3ds;
	else if( ext=="b3d" ) l=&loader_b3d;
	else return 0;

	const Transform &conv=loader_mat_map[ext];

	CachedTexture::setPath( filenamepath( t ) );
	Entity *e=l->load( t,conv,hint );
	CachedTexture::setPath( "" );
	return e;
}

static void collapseMesh( MeshModel *mesh,Entity *e ){
	while( e->children() ){
		collapseMesh( mesh,e->children() );
	}
	if( Model *p=e->getModel() ){
		if( MeshModel *t=p->getMeshModel() ){
			t->transform( e->getWorldTform() );
			mesh->add( *t );
		}
	}
	delete e;
}

static void insert( Entity *e ){
	entity_set.insert( e );
	e->setVisible(true);
	e->setEnabled(true);
	e->getObject()->reset();
	for( Entity *p=e->children();p;p=p->successor() ){
		insert( p );
	}
}

static Entity *insertEntity( Entity *e,Entity *p ){
	e->setParent( p );
	insert( e );
	return e;
}

static void erase( Entity *e ){
	for( Entity *p=e->children();p;p=p->successor() ){
		erase( p );
	}
	if( e->getListener() ) listener=0;
	entity_set.erase( e );
}

static Entity *findChild( Entity *e,const string &t ){
	if( e->getName()==t ) return e;
	for( Entity *p=e->children();p;p=p->successor() ){
		if( Entity *q=findChild(p,t) ) return q;
	}
	return 0;
}

///////////////////////////
// GLOBAL WORLD COMMANDS //
///////////////////////////
void  bbLoaderMatrix( BBStr *ext,float xx,float xy,float xz,float yx,float yy,float yz,float zx,float zy,float zz ){
	loader_mat_map.erase( *ext );
	loader_mat_map[*ext]=Transform(Matrix(Vector(xx,xy,xz),Vector(yx,yy,yz),Vector(zx,zy,zz)));
	delete ext;
}

int   bbHWTexUnits(){
	if (!debug3d("HWTexUnits")) return 0;
	return gx_scene->hwTexUnits();
}

int	  bbGfxDriverCaps3D(){
	if (!debug3d("GfxDriverCaps3D")) return 0;
	return gx_scene->gfxDriverCaps3D();
}

void  bbHWMultiTex( int enable ){
	if (!debug3d("HWMultiTex")) return;
	gx_scene->setHWMultiTex( !!enable );
}

void  bbWBuffer( int enable ){
	if (!debug3d("WBuffer")) return;
	gx_scene->setWBuffer( !!enable );
}

void  bbDither( int enable ){
	if (!debug3d("Dither")) return;
	gx_scene->setDither( !!enable );
}

void  bbAntiAlias( int enable ){
	if (!debug3d("AntiAlias")) return;
	gx_scene->setAntialias( !!enable );
}

void  bbWireFrame( int enable ){
	if (!debug3d("WireFrame")) return;
	gx_scene->setWireframe( !!enable );
}

void  bbAmbientLight( float r,float g,float b ){
	if (!debug3d("AmbientLight")) return;
	Vector t( r*ctof,g*ctof,b*ctof );
	gx_scene->setAmbient( &(t.x) );
}

void  bbClearCollisions(){
	if (!debug3d("ClearCollisions")) return;
	world->clearCollisions();
}

void  bbCollisions( int src_type,int dest_type,int method,int response ){
	if (!debug3d("Collisions")) return;
	world->addCollision( src_type,dest_type,method,response );
}

static int update_ms;

void  bbUpdateWorld( float elapsed ){
	if (!debug3d("UpdateWorld")) return;

#ifndef BETA
	world->update( elapsed );
	return;
#endif

	update_ms=gx_runtime->getMilliSecs();
	world->update( elapsed );
	update_ms=gx_runtime->getMilliSecs()-update_ms;
}

void  bbCaptureWorld(){
	if (!debug3d("CaptureWorld")) return;
	world->capture();
}

void  bbRenderWorld( float tween ){
	if (!debug3d("RenderWorld")) return;

#ifndef BETA
	tri_count=gx_scene->getTrianglesDrawn();
	world->render( tween );
	tri_count=gx_scene->getTrianglesDrawn()-tri_count;
	return;
#endif

	int tris=gx_scene->getTrianglesDrawn();
	int render_ms=gx_runtime->getMilliSecs();
	world->render( tween );
	render_ms=gx_runtime->getMilliSecs()-render_ms;

	extern int bbKeyHit(int);
	extern void bbDelay(int);
	bbDelay(0);
	if( bbKeyHit( 0x57 ) ){
		stats_mode=!stats_mode;
	}
	if( bbKeyHit( 0x58 ) ){
		static int n;
		string t="screenshot"+itoa(++n)+".bmp";
		bbSaveBuffer( bbBackBuffer(),d_new BBStr(t) );
	}

	if( !stats_mode ) return;

	tris=gx_scene->getTrianglesDrawn()-tris;

	static int time;
	int frame_ms=gx_runtime->getMilliSecs()-time;
	time+=frame_ms;

	int fps=frame_ms ? 1000/frame_ms : 1000;
	int ups=update_ms ? 1000/update_ms : 1000;
	int rps=render_ms ? 1000/render_ms : 1000;

	string t_fps="000"+itoa(fps);t_fps=t_fps.substr( t_fps.size()-4 );
	string t_ups="000"+itoa(ups);t_ups=t_ups.substr( t_ups.size()-4 );
	string t_rps="000"+itoa(rps);t_rps=t_rps.substr( t_rps.size()-4 );
	string t_tris="00000"+itoa(tris);t_tris=t_tris.substr( t_tris.size()-6 );

	string t="FPS:"+t_fps+" UPS:"+t_ups+" RPS:"+t_rps+" TRIS:"+t_tris;

	bbText( 0,bbGraphicsHeight()-bbFontHeight(),d_new BBStr(t),0,0 );
}

int  bbTrisRendered(){
	return tri_count;
}

float  bbStats3D( int n ){
	return stats3d[n];
}

//////////////////////
// TEXTURE COMMANDS //
//////////////////////

//Note: modify canvas->backup() to NOT release backup image!
//
Texture *  bbLoadTexture( BBStr *file,int flags ){
	if (!debug3d("LoadTexture")) return 0;
	Texture *t=d_new Texture( *file,flags );delete file;
	if( !t->getCanvas(0) ){ delete t;return 0; }
	texture_set.insert( t );
	return t;
}

Texture *  bbLoadAnimTexture( BBStr *file,int flags,int w,int h,int first,int cnt ){
	if (!debug3d("LoadAnimTexture")) return 0;
	Texture *t=d_new Texture( *file,flags,w,h,first,cnt );
	delete file;
	if( !t->getCanvas(0) ){
		delete t;
		return 0; 
	}
	texture_set.insert( t );
	return t;
}

Texture *  bbCreateTexture( int w,int h,int flags,int frames ){
	/*if( debug ){
		if (!debug3d("CreateTexture")) RTEX("3D Graphics Mode not set");
		if( frames<=0 ){
			RTEX( "Illegal number of texture frames" );
		}
	}*/
	if (!debug3d("CreateTexture")) return 0;
	if ( frames<=0 ){
		if (debug) {
			RTEX( "Illegal number of texture frames" );
		} else {
			errorLog.push_back(std::string("CreateTexture: Illegal number of texture frames"));
		}
	}
	Texture *t=d_new Texture( w,h,flags,frames );
	texture_set.insert( t );
	return t;
}

void  bbFreeTexture( Texture *t ){
	if( !t ) return;
	if (!debugTexture(t,"FreeTexture")) return;
	if( texture_set.erase( t ) ) delete t;
}

void  bbTextureBlend( Texture *t,int blend ){
	if (!debugTexture(t,"TextureBlend")) return;
	t->setBlend( blend );
}

void  bbTextureCoords( Texture *t,int flags ){
	if (!debugTexture(t,"TextureCoords")) return;
	t->setFlags( flags );
}

void  bbTextureBumpEnvMat( Texture* t,int x,int y,float envmat ){
	if (!debugTexture(t,"TextureBumpEnvMat")) return;
	t->setBumpEnvMat(x,y,envmat);
}

void  bbTextureBumpEnvScale( Texture* t,float envscale ){
	if (!debugTexture(t,"TextureBumpEnvScale")) return;
	t->setBumpEnvScale(envscale);
}

void  bbTextureBumpEnvOffset( Texture* t,float envoffset ){
	if (!debugTexture(t,"TextureBumpEnvOffset")) return;
	t->setBumpEnvOffset(envoffset);
}

void  bbScaleTexture( Texture *t,float u_scale,float v_scale ){
	if (!debugTexture(t,"ScaleTexture")) return;
	t->setScale( 1/u_scale,1/v_scale );
}

void  bbRotateTexture( Texture *t,float angle ){
	if (!debugTexture(t,"RotateTexture")) return;
	t->setRotation( -angle*dtor );
}

void  bbPositionTexture( Texture *t,float u_pos,float v_pos ){
	if (!debugTexture(t,"PositionTexture")) return;
	t->setPosition( -u_pos,-v_pos );
}

void  bbTextureLodBias( float bias ){
	gx_scene->textureLodBias = *((DWORD*)&bias);
}

int  bbTextureWidth( Texture *t ){
	if (!debugTexture(t,"TextureWidth")) return 0;
	return t->getCanvas(0)->getWidth();
}

int  bbTextureHeight( Texture *t ){
	if (!debugTexture(t,"TextureHeight")) return 0;
	return t->getCanvas(0)->getHeight();
}

BBStr *bbTextureName( Texture *t ){
	if (!debugTexture(t,"TextureName")) return d_new BBStr("");
	CachedTexture *c=t->getCachedTexture();
	return c ? d_new BBStr( c->getName().c_str() ) : d_new BBStr("");
}

void bbSetCubeFace( Texture *t,int face ){
	if (!debugTexture(t,"SetCubeFace")) return;
	if( gxCanvas *c=t->getCanvas( 0 ) ){
		c->setCubeFace(face);
	}
}

void bbSetCubeMode( Texture *t,int mode ){
	if (!debugTexture(t,"SetCubeMode")) return;
	if( gxCanvas *c=t->getCanvas( 0 ) ){
		c->setCubeMode( mode );
	}
}

gxCanvas *bbTextureBuffer( Texture *t,int frame ){
	//v1.04
	if (!debugTexture(t,"TextureBuffer")) return 0;
	if( gxCanvas *c=t->getCanvas( frame ) ){
		if( c->getDepth() ) return c;
	}
	return 0;
	/*
	//v1.03  crashes if t->getCanvas returns null!
	debugTexture(t);
	gxCanvas *c=t->getCanvas( frame );
	if( c->getDepth() ) return c;
	return 0;
	*/
}

void  bbClearTextureFilters(){
	if (!debug3d("ClearTextureFilters")) return;
	Texture::clearFilters();
}

void  bbTextureFilter( BBStr *t,int flags ){
	if (!debug3d("TextureFilter")) return;
	Texture::addFilter( *t,flags );
	delete t;
}

////////////////////
// BRUSH COMMANDS //
////////////////////
Brush *  bbCreateBrush( float r,float g,float b ){
	if (!debug3d("CreateBrush")) return 0;
	Brush *br=d_new Brush();
	br->setColor( Vector( r*ctof,g*ctof,b*ctof ) );
	brush_set.insert( br );
	return br;
}

Brush *  bbLoadBrush( BBStr *file,int flags,float u_scale,float v_scale ){
	if (!debug3d("LoadBrush")) return 0;
	Texture t( *file,flags );
	delete file;if( !t.getCanvas(0) ) return 0;
	if( u_scale!=1 || v_scale!=1 ) t.setScale( 1/u_scale,1/v_scale );
	Brush *br=bbCreateBrush( 255,255,255 );
	br->setTexture( 0,t,0 );
	delete file;
	return br;
}

void  bbFreeBrush( Brush *b ){
	if( !b ) return;
	if (!debugBrush(b,"FreeBrush")) return;
	if( brush_set.erase( b ) ) delete b;
}

void  bbBrushColor( Brush *br,float r,float g,float b ){
	if (!debugBrush(br,"BrushColor")) return;
	br->setColor( Vector( r*ctof,g*ctof,b*ctof ) );
}

void  bbBrushAlpha( Brush *b,float alpha ){
	if (!debugBrush(b,"BrushAlpha")) return;
	b->setAlpha( alpha );
}

void  bbBrushShininess( Brush *b,float n ){
	if (!debugBrush(b,"BrushShininess")) return;
	b->setShininess( n );
}

void  bbBrushTexture( Brush *b,Texture *t,int frame,int index ){
	if (!debugBrush(b,"BrushTexture")) return;
	if (!debugTexture(t,"BrushTexture")) return;
	b->setTexture( index,*t,frame );
}

Texture *bbGetBrushTexture( Brush *b,int index ){
	if (!debugBrush(b,"GetBrushTexture")) return 0;
	Texture *tex=d_new Texture(b->getTexture(index));
	texture_set.insert( tex );
	return tex;
}

void  bbBrushBlend( Brush *b,int blend ){
	if (!debugBrush(b,"BrushBlend")) return;
	b->setBlend( blend );
}

void  bbBrushFX( Brush *b,int fx ){
	if (!debugBrush(b,"BrushFX")) return;
	b->setFX( fx );
}

///////////////////
// MESH COMMANDS //
///////////////////
Entity *  bbCreateMesh( Entity *p ){
	if (!debugParent(p,"CreateMesh")) return 0;
	MeshModel *m=d_new MeshModel();
	return insertEntity( m,p );
}

Entity *  bbLoadMesh( BBStr *f,Entity *p ){
	if (!debugParent(p,"LoadMesh")) return 0;
	Entity *e=loadEntity( f->c_str(),MeshLoader::HINT_COLLAPSE );
	delete f;

	if( !e ) return 0;
	MeshModel *m=d_new MeshModel();
	collapseMesh( m,e );
	return insertEntity( m,p );
}

Entity *  bbLoadAnimMesh( BBStr *f,Entity *p ){
	if (!debugParent(p,"LoadAnimMesh")) return 0;
	Entity *e=loadEntity( f->c_str(),0 );
	delete f;

	if( !e ) return 0;
	if( Animator *anim=e->getObject()->getAnimator() ){
		anim->animate( 1,0,0,0 );
	}
	return insertEntity( e,p );
}

Entity *  bbCreateCube( Entity *p ){
	if (!debugParent(p,"CreateCube")) return 0;
	Entity *e=MeshUtil::createCube( Brush() );
	return insertEntity( e,p );
}

Entity *  bbCreateSphere( int segs,Entity *p ){
	if (!debugParent(p,"CreateSphere")) return 0;
	if( debug ){
		if( segs<2 || segs>100 ) RTEX( "Illegal number of segments" );
	} else {
		if( segs<2 || segs>100 ) {
			errorLog.push_back(std::string("CreateSphere: Illegal number of segments"));
			return 0;
		}
	}
	Entity *e=MeshUtil::createSphere( Brush(),segs );
	return insertEntity( e,p );
}

Entity *  bbCreateCylinder( int segs,int solid,Entity *p ){
	if (!debugParent(p,"CreateCylinder")) return 0;
	if( debug ){
		if( segs<3 || segs>100 ) RTEX( "Illegal number of segments" );
	} else {
		if( segs<3 || segs>100 ) {
			errorLog.push_back(std::string("CreateCylinder: Illegal number of segments"));
			return 0;
		}
	}
	Entity *e=MeshUtil::createCylinder( Brush(),segs,!!solid );
	return insertEntity( e,p );
}

Entity *  bbCreateCone( int segs,int solid,Entity *p ){
	if (!debugParent(p,"CreateCone")) return 0;
	if( debug ){
		if( segs<3 || segs>100 ) RTEX( "Illegal number of segments" );
	} else {
		if( segs<3 || segs>100 ) {
			errorLog.push_back(std::string("CreateCone: Illegal number of segments"));
			return 0;
		}
	}
	Entity *e=MeshUtil::createCone( Brush(),segs,!!solid );
	return insertEntity( e,p );
}

Entity *  bbCopyMesh( MeshModel *m,Entity *p ){
	if (!debugMesh(m,"CopyMesh")) return 0;
	if (!debugParent(p,"CopyMesh")) return 0;

	MeshModel *t=d_new MeshModel();
	t->add( *m );
	return insertEntity( t,p );
}

void  bbScaleMesh( MeshModel *m,float x,float y,float z ){
	if (!debugMesh(m,"ScaleMesh")) return;
	m->transform( scaleMatrix(x,y,z) );
}

void  bbRotateMesh( MeshModel *m,float x,float y,float z ){
	if (!debugMesh(m,"RotateMesh")) return;
	m->transform( rotationMatrix(x*dtor,y*dtor,z*dtor) );
}

void  bbPositionMesh( MeshModel *m,float x,float y,float z ){
	if (!debugMesh(m,"PositionMesh")) return;
	m->transform( Vector(x,y,z) );
}

void  bbFitMesh( MeshModel *m,float x,float y,float z,float w,float h,float d,int uniform ){
	if (!debugMesh(m,"FitMesh")) return;
	Box box( Vector(x,y,z) );
	box.update( Vector(x+w,y+h,z+d) );
	const Box &curr_box=m->getBox();
	float x_scale=box.width()/curr_box.width();
	float y_scale=box.height()/curr_box.height();
	float z_scale=box.depth()/curr_box.depth();
	Transform t;
	if( uniform ){
		if( x_scale<y_scale && x_scale<z_scale ){
			y_scale=z_scale=x_scale;
		}else if( y_scale<x_scale && y_scale<z_scale ){
			x_scale=z_scale=y_scale;
		}else{
			x_scale=y_scale=z_scale;
		}
	}
	t.m.i.x=x_scale;
	t.m.j.y=y_scale;
	t.m.k.z=z_scale;
	t.v=box.centre() - t.m * curr_box.centre();
	m->transform( t );
}

void  bbFlipMesh( MeshModel *m ){
	if (!debugMesh(m,"FlipMesh")) return;
	m->flipTriangles();
}

void  bbPaintMesh( MeshModel *m,Brush *b ){
	//if( debug ){ debugMesh(m);debugBrush(b); }
	if (!debugMesh(m,"PaintMesh")) return;
	if (!debugBrush(b,"PaintMesh")) return;
	m->paint( *b );
}

void  bbAddMesh( MeshModel *src,MeshModel *dest ){
	if (!debugMesh(src,"AddMesh (src)")) return;
	if (!debugMesh(dest,"AddMesh (dest)")) return;
	if( debug ){
		if( src==dest ) RTEX( "A mesh cannot be added to itself" );
	} else {
		if (src==dest) {
			errorLog.push_back(std::string("AddMesh: A mesh cannot be added to itself"));
			return;
		}
	}

	dest->add( *src );
}

void  bbUpdateNormals( MeshModel *m ){
	if (!debugMesh(m,"UpdateNormals")) return;
	m->updateNormals();
}

void  bbLightMesh( MeshModel *m,float r,float g,float b,float range,float x,float y,float z ){
	if (!debugMesh(m,"LightMesh")) return;
	MeshUtil::lightMesh( m,Vector(x,y,z),Vector(r*ctof,g*ctof,b*ctof),range );
}

float  bbMeshWidth( MeshModel *m ){
	if (!debugMesh(m,"MeshWidth")) return 0;
	return m->getBox().width();
}

float  bbMeshHeight( MeshModel *m ){
	if (!debugMesh(m,"MeshHeight")) return 0;
	return m->getBox().height();
}

float  bbMeshDepth( MeshModel *m ){
	if (!debugMesh(m,"MeshDepth")) return 0;
	return m->getBox().depth();
}

int  bbMeshesIntersect( MeshModel *a,MeshModel *b ){
	//if( debug ){ debugMesh(a);debugMesh(b); }
	if (!debugMesh(a,"MeshesIntersect (a)")) return 0;
	if (!debugMesh(b,"MeshesIntersect (b)")) return 0;
	return a->intersects( *b );
}

int  bbCountSurfaces( MeshModel *m ){
	if (!debugMesh(m,"CountSurfaces")) return 0;
	return m->getSurfaces().size();
}

Surface *  bbGetSurface( MeshModel *m,int index ){
	if (!debugMesh(m,"GetSurface")) return 0;
	if( debug ){
		if( index<1 || index>m->getSurfaces().size() ){
			RTEX( "Surface Index out of range" );
		}
	} else {
		if( index<1 || index>m->getSurfaces().size() ){
			errorLog.push_back("GetSurface: Surface Index out of range");
			return 0;
		}
	}
	return m->getSurfaces()[index-1];
}

void bbMeshCullBox( MeshModel *m,float x,float y,float z,float width,float height,float depth ){
	//if( debug ){
	if (!debugMesh(m,"MeshCullBox")) return;
	//}
	m->setCullBox( Box( Vector(x,y,z),Vector(x+width,y+height,z+depth) ) );
}


//////////////////////
// SURFACE COMMANDS //
//////////////////////
Surface *  bbFindSurface( MeshModel *m,Brush *b ){
	//if( debug ){ debugMesh(m);debugBrush(b); }
	if (!debugMesh(m,"FindSurface")) return 0;
	if (!debugBrush(b,"FindSurface")) return 0;
	return m->findSurface(*b);
}

Surface *  bbCreateSurface( MeshModel *m,Brush *b ){
	//if( debug ){ debugMesh(m);if( b ) debugBrush(b); }
	if (!debugMesh(m,"CreateSurface")) return 0;
	if (b) { if (!debugBrush(b,"CreateSurface")) return 0; }
	Surface *s=b ? m->createSurface( *b ) : m->createSurface( Brush() );
	return s;
}

Brush *bbGetSurfaceBrush( Surface *s ){
	Brush *br=d_new Brush( s->getBrush() );
	brush_set.insert( br );
	return br;
}

Brush *bbGetEntityBrush( Model *m ){
	if (!debugModel(m,"GetEntityBrush")) return 0;
	Brush *br=d_new Brush( m->getBrush() );
	brush_set.insert( br );
	return br;
}

void  bbClearSurface( Surface *s,int verts,int tris ){
	s->clear( !!verts,!!tris );
}

void  bbPaintSurface( Surface *s,Brush *b ){
	if (!debugBrush(b,"PaintSurface")) return;
	s->setBrush(*b);
}

int  bbAddVertex( Surface *s,float x,float y,float z,float tu,float tv,float tw ){
	Surface::Vertex v;
	v.coords=Vector(x,y,z);
	v.color=0xffffffff;
	v.tex_coords[0][0]=v.tex_coords[1][0]=tu;
	v.tex_coords[0][1]=v.tex_coords[1][1]=tv;
	s->addVertex( v );
	return s->numVertices()-1;
}

int  bbAddTriangle( Surface *s,int v0,int v1,int v2 ){
	Surface::Triangle t;
	t.verts[0]=v0;t.verts[1]=v1;t.verts[2]=v2;
	s->addTriangle( t );
	return s->numTriangles()-1;
}

void  bbVertexCoords( Surface *s,int n,float x,float y,float z ){
	s->setCoords( n,Vector(x,y,z) );
}

void  bbVertexNormal( Surface *s,int n,float x,float y,float z ){
	s->setNormal( n,Vector(x,y,z) );
}

void  bbVertexColor( Surface *s,int n,float r,float g,float b,float a ){
	if(r<0)r=0;else if(r>255)r=255;
	if(g<0)g=0;else if(g>255)g=255;
	if(b<0)b=0;else if(b>255)b=255;
	a*=255;if(a<0)a=0;else if(a>255)a=255;
	s->setColor( n,(int(a)<<24)|(int(r)<<16)|(int(g)<<8)|int(b) );
}

void  bbVertexTexCoords( Surface *s,int n,float u,float v,float w,int set ){
	s->setTexCoords( n,Vector(u,v,w),set );
}

int  bbCountVertices( Surface *s ){
	return s->numVertices();
}

int  bbCountTriangles( Surface *s ){
	return s->numTriangles();
}

float  bbVertexX( Surface *s,int n ){
	if (!debugVertex(s,n,"VertexX")) return 0;
	return s->getVertex(n).coords.x;
}
float  bbVertexY( Surface *s,int n ){
	if (!debugVertex(s,n,"VertexY")) return 0;
	return s->getVertex(n).coords.y;
}
float  bbVertexZ( Surface *s,int n ){
	if (!debugVertex(s,n,"VertexZ")) return 0;
	return s->getVertex(n).coords.z;
}
float  bbVertexNX( Surface *s,int n ){
	if (!debugVertex(s,n,"VertexNX")) return 0;
	return s->getVertex(n).normal.x;
}
float  bbVertexNY( Surface *s,int n ){
	if (!debugVertex(s,n,"VertexNY")) return 0;
	return s->getVertex(n).normal.y;
}
float  bbVertexNZ( Surface *s,int n ){
	if (!debugVertex(s,n,"VertexNZ")) return 0;
	return s->getVertex(n).normal.z;
}
float  bbVertexRed( Surface *s,int n ){
	if (!debugVertex(s,n,"VertexRed")) return 0;
	return (s->getVertex(n).color&0xff0000)>>16;
}
float  bbVertexGreen( Surface *s,int n ){
	if (!debugVertex(s,n,"VertexGreen")) return 0;
	return (s->getVertex(n).color&0xff00)>>8;
}
float  bbVertexBlue( Surface *s,int n ){
	if (!debugVertex(s,n,"VertexBlue")) return 0;
	return s->getVertex(n).color&0xff;
}
float  bbVertexAlpha( Surface *s,int n ){
	if (!debugVertex(s,n,"VertexAlpha")) return 0;
	return ((s->getVertex(n).color&0xff000000)>>24)/255.0f;
}
float  bbVertexU( Surface *s,int n,int t ){
	if (!debugVertex(s,n,t,"VertexU")) return 0;
	return s->getVertex(n).tex_coords[t][0];
}
float  bbVertexV( Surface *s,int n,int t ){
	if (!debugVertex(s,n,t,"VertexV")) return 0;
	return s->getVertex(n).tex_coords[t][1];
}
float  bbVertexW( Surface *s,int n,int t ){
	if (!debugVertex(s,n,t,"VertexW")) return 0;
	return 1;
}
int  bbTriangleVertex( Surface *s,int n,int v ){
	return s->getTriangle(n).verts[v];
}

/////////////////////
// CAMERA COMMANDS //
/////////////////////
Entity *  bbCreateCamera( Entity *p ){
	if (!debugParent(p,"CreateCamera")) return 0;
	int x,y,w,h;
	gx_canvas->getViewport( &x,&y,&w,&h );
	Camera *c=d_new Camera();
	c->setViewport( x,y,w,h );
	return insertEntity( c,p );
}

void  bbCameraZoom( Camera *c,float zoom ){
	if (!debugCamera(c,"CameraZoom")) return;
	c->setZoom( zoom );
}

void  bbCameraRange( Camera *c,float nr,float fr ){
	if (!debugCamera(c,"CameraRange")) return;
	c->setRange( nr,fr );
}

void  bbCameraClsColor( Camera *c,float r,float g,float b ){
	if (!debugCamera(c,"CameraClsColor")) return;
	c->setClsColor( Vector( r*ctof,g*ctof,b*ctof ) );
}

void  bbCameraClsMode( Camera *c,int cls_color,int cls_zbuffer ){
	if (!debugCamera(c,"CameraClsMode")) return;
	c->setClsMode( cls_color ? true : false,cls_zbuffer ? true : false );
}

void  bbCameraProjMode( Camera *c,int mode ){
	if (!debugCamera(c,"CameraProjMode")) return;
	c->setProjMode( mode );
}

void  bbCameraViewport( Camera *c,int x,int y,int w,int h ){
	if (!debugCamera(c,"CameraViewport")) return;
	c->setViewport( x,y,w,h );
}

void  bbCameraFogRange( Camera *c,float nr,float fr ){
	if (!debugCamera(c,"CameraFogRange")) return;
	c->setFogRange( nr,fr );
}

void  bbCameraFogColor( Camera *c,float r,float g,float b ){
	if (!debugCamera(c,"CameraFogColor")) return;
	c->setFogColor( Vector( r*ctof,g*ctof,b*ctof ) );
}

void  bbCameraFogMode( Camera *c,int mode ){
	if (!debugCamera(c,"CameraFogMode")) return;
	c->setFogMode( mode );
}

int  bbCameraProject( Camera *c,float x,float y,float z ){
	if (!debugCamera(c,"CameraProject")) return 0;
	Vector v=-c->getWorldTform()*Vector(x,y,z);
	const Frustum &f=c->getFrustum();
	if( c->getProjMode()==Camera::PROJ_ORTHO ){
		int vp_x,vp_y,vp_w,vp_h;
		c->getViewport( &vp_x,&vp_y,&vp_w,&vp_h );
		float nr=c->getFrustumNear();
		float fr=c->getFrustumFar();
		float nr_w=c->getFrustumWidth();
		float nr_h=c->getFrustumHeight();
		projected=Vector( (v.x/nr_w+.5f)*vp_w,(.5f-v.y/nr_h)*vp_h,nr );
		return 1;
	}
	if( v.z>0 ){
		float fr=+f.getPlane( Frustum::PLANE_FAR ).d;
		if( v.z<=fr ){
			int vp_x,vp_y,vp_w,vp_h;
			c->getViewport( &vp_x,&vp_y,&vp_w,&vp_h );
			float nr=c->getFrustumNear();
			float fr=c->getFrustumFar();
			float nr_w=c->getFrustumWidth();
			float nr_h=c->getFrustumHeight();
			projected=Vector( 
				(v.x*nr/v.z/nr_w+.5f)*vp_w,
				(.5f-v.y*nr/v.z/nr_h)*vp_h,nr );
			return 1;
		}
	}
	projected=Vector();
	return 0;
}

float  bbProjectedX(){
	return projected.x;
}

float  bbProjectedY(){
	return projected.y;
}

float  bbProjectedZ(){
	return projected.z;
}

static Object *doPick( const Line &l,float radius ){
	picked.collision.time=1;
	return world->traceRay( l,radius,&picked );
}

Entity *  bbCameraPick( Camera *c,float x,float y ){
	if (!debugCamera(c,"CameraPick")) return 0;

	int vp_x,vp_y,vp_w,vp_h;
	c->getViewport( &vp_x,&vp_y,&vp_w,&vp_h );
	float nr=c->getFrustumNear();
	float fr=c->getFrustumFar();
	float nr_w=c->getFrustumWidth();
	float nr_h=c->getFrustumHeight();

	x=((x/vp_w)-.5f)*nr_w;
	y=(.5f-(y/vp_h))*nr_h;

	Line l;
	if( c->getProjMode()==Camera::PROJ_ORTHO ){
		l=c->getWorldTform() * Line( Vector(x,y,0),Vector(0,0,fr) );	//x,y,fr) );
	}else{
		x/=nr;y/=nr;
		l=c->getWorldTform() * Line( Vector(),Vector( x*fr,y*fr,fr ) );
	}

	return doPick( l,0 );
}

Entity *  bbLinePick( float x,float y,float z,float dx,float dy,float dz,float radius ){
	if (!debug3d("LinePick")) return 0;

	Line l( Vector( x,y,z ),Vector( dx,dy,dz ) );

	return doPick( l,radius );
}

Entity *  bbEntityPick( Object *src,float range ){
	if (!debugEntity(src,"EntityPick")) return 0;

	Line l( src->getWorldPosition(),src->getWorldTform().m.k * range );

	return doPick( l,0 );
}

int  bbEntityVisible( Object *src,Object *dest ){
	//if( debug ){ debugObject(src);debugObject(dest); }
	if (!debugObject(src,"EntityVisible (src)")) return 0;
	if (!debugObject(dest,"EntityVisible (dest)")) return 0;
	
	return world->checkLOS( src,dest ) ? 1 : 0;
}

int  bbEntityInView( Entity *e,Camera *c ){
	//if( debug ){ debugEntity(e);debugCamera(c); }
	if (!debugEntity(e,"EntityInView")) return 0;
	if (!debugCamera(c,"EntityInView")) return 0;
	if( Model *p=e->getModel() ){
		if( MeshModel *m=p->getMeshModel() ){
			const Box &b=m->getBox();
			Transform t=-c->getWorldTform() * e->getWorldTform();
			Vector p[]={ 
				t*b.corner(0),t*b.corner(1),t*b.corner(2),t*b.corner(3),
				t*b.corner(4),t*b.corner(5),t*b.corner(6),t*b.corner(7)
			};
			return c->getFrustum().cull( p,8 );
		}
	}
	Vector p[]={ -c->getWorldTform() * e->getWorldPosition() };
	return c->getFrustum().cull( p,1 );
}

float  bbPickedX(){
	return picked.coords.x;
}

float  bbPickedY(){
	return picked.coords.y;
}

float  bbPickedZ(){
	return picked.coords.z;
}

float  bbPickedNX(){
	return picked.collision.normal.x;
}

float  bbPickedNY(){
	return picked.collision.normal.y;
}

float  bbPickedNZ(){
	return picked.collision.normal.z;
}

float  bbPickedTime(){
	return picked.collision.time;
}

Object * bbPickedEntity(){
	return picked.with;
}

void * bbPickedSurface(){
	return picked.collision.surface;
}

int  bbPickedTriangle(){
	return picked.collision.index;
}

////////////////////
// LIGHT COMMANDS //
////////////////////
Entity *  bbCreateLight( int type,Entity *p ){
	if (!debugParent(p,"CreateLight")) return 0;
	Light *t=d_new Light( type );
	return insertEntity( t,p );
}

void  bbLightColor( Light *light,float r,float g,float b ){
	if (!debugLight(light,"LightColor")) return;
	light->setColor( Vector(r*ctof,g*ctof,b*ctof) );
}

void  bbLightRange( Light *light,float range ){
	if (!debugLight(light,"LightRange")) return;
	light->setRange( range );
}

void  bbLightConeAngles( Light *light,float inner,float outer ){
	if (!debugLight(light,"LightConeAngles")) return;
	inner*=dtor;
	outer*=dtor;
	if( inner<0 ) inner=0;
	else if( inner>PI ) inner=PI;
	if( outer<inner ) outer=inner;
	else if( outer>PI ) outer=PI;
	light->setConeAngles( inner,outer );
}

////////////////////
// PIVOT COMMANDS //
////////////////////
Entity *  bbCreatePivot( Entity *p ){
	if (!debugParent(p,"CreatePivot")) return 0;
	Pivot *t=d_new Pivot();
	return insertEntity( t,p );
}

/////////////////////
// SPRITE COMMANDS //
/////////////////////
Entity *  bbCreateSprite( Entity *p ){
	if (!debugParent(p,"CreateSprite")) return 0;
	Sprite *s=d_new Sprite();
	s->setFX( gxScene::FX_FULLBRIGHT );
	return insertEntity( s,p );
}

Entity *  bbLoadSprite( BBStr *file,int flags,Entity *p ){
	if (!debugParent(p,"LoadSprite")) return 0;
	Texture t( *file,flags );
	delete file;if( !t.getCanvas(0) ) return 0;
	Sprite *s=d_new Sprite();
	s->setTexture( 0,t,0 );
	s->setFX( gxScene::FX_FULLBRIGHT );

	if( flags & gxCanvas::CANVAS_TEX_MASK ) s->setBlend( gxScene::BLEND_REPLACE );
	else if( flags & gxCanvas::CANVAS_TEX_ALPHA ) s->setBlend( gxScene::BLEND_ALPHA );
	else s->setBlend( gxScene::BLEND_ADD );

	return insertEntity( s,p );
}

void  bbRotateSprite( Sprite *s,float angle ){
	if (!debugSprite(s,"RotateSprite")) return;
	s->setRotation( angle*dtor );
}

void  bbScaleSprite( Sprite *s,float x,float y ){
	if (!debugSprite(s,"ScaleSprite")) return;
	s->setScale( x,y );
}

void  bbHandleSprite( Sprite *s,float x,float y ){
	if (!debugSprite(s,"HandleSprite")) return;
	s->setHandle( x,y );
}

void  bbSpriteViewMode( Sprite *s,int mode ){
	if (!debugSprite(s,"SpriteViewMode")) return;
	s->setViewmode( mode );
}

/////////////////////
// MIRROR COMMANDS //
/////////////////////
Entity *  bbCreateMirror( Entity *p ){
	if (!debugParent(p,"CreateMirror")) return 0;
	Mirror *t=d_new Mirror();
	return insertEntity( t,p );
}

////////////////////
// PLANE COMMANDS //
////////////////////
Entity *  bbCreatePlane( int segs,Entity *p ){
	if (!debugParent(p,"CreatePlane")) return 0;
	if( debug ){
		if( segs<1 || segs>20 ) RTEX( "Illegal number of segments" );
	} else {
		if( segs<1 || segs>20 ) {
			errorLog.push_back(std::string("CreatePlane: Illegal number of segments"));
			return 0;
		}
	}
	PlaneModel *t=d_new PlaneModel( segs );
	return insertEntity( t,p );
}

//////////////////
// MD2 COMMANDS //
//////////////////
Entity *  bbLoadMD2( BBStr *file,Entity *p ){
	if (!debugParent(p,"LoadMD2")) return 0;
	MD2Model *t=d_new MD2Model( *file );delete file;
	if( !t->getValid() ){ delete t;return 0; }
	return insertEntity( t,p );
}

void  bbAnimateMD2( MD2Model *m,int mode,float speed,int first,int last,float trans ){
	if (!debugMD2(m,"AnimateMD2")) return;
	m->startMD2Anim( first,last,mode,speed,trans );
}

float  bbMD2AnimTime( MD2Model *m ){
	if (!debugMD2(m,"MD2AnimTime")) return 0;
	return m->getMD2AnimTime();
}

int  bbMD2AnimLength( MD2Model *m ){
	if (!debugMD2(m,"MD2AnimLength")) return 0;
	return m->getMD2AnimLength();
}

int  bbMD2Animating( MD2Model *m ){
	if (!debugMD2(m,"MD2Animating")) return 0;
	return m->getMD2Animating();
}

//////////////////
// BSP Commands //
//////////////////
Entity *  bbLoadBSP( BBStr *file,float gam,Entity *p ){
	if (!debugParent(p,"LoadBSP")) return 0;
	CachedTexture::setPath( filenamepath( *file ) );
	Q3BSPModel *t=d_new Q3BSPModel( *file,gam );delete file;
	CachedTexture::setPath( "" );

	if( !t->isValid() ){ delete t;return 0; }

	return insertEntity( t,p );
}

void  bbBSPAmbientLight( Q3BSPModel *t,float r,float g,float b ){
	if (!debugBSP(t,"BSPAmbientLight")) return;
	t->setAmbient( Vector( r*ctof,g*ctof,b*ctof ) );
}

void  bbBSPLighting( Q3BSPModel *t,int lmap ){
	if (!debugBSP(t,"BSPLighting")) return;
	t->setLighting( !!lmap );
}

//////////////////////
// TERRAIN COMMANDS //
//////////////////////
static float terrainHeight( Terrain *t,float x,float z ){
	int ix=floor(x);
	int iz=floor(z);
	float tx=x-ix,tz=z-iz;
	float h0=t->getHeight(ix,iz);
	float h1=t->getHeight(ix+1,iz);
	float h2=t->getHeight(ix,iz+1);
	float h3=t->getHeight(ix+1,iz+1);
	float ha=(h1-h0)*tx+h0,hb=(h3-h2)*tx+h2;
	float h=(hb-ha)*tz+ha;
	return h;
}

static Vector terrainVector( Terrain *t,float x,float y,float z ){
	Vector v=-t->getWorldTform() * Vector( x,y,z );
	return t->getWorldTform() * Vector( v.x,terrainHeight( t,v.x,v.z ),v.z );
}

Entity *  bbCreateTerrain( int n,Entity *p ){
	if (!debugParent(p,"CreateTerrain")) return 0;
	int shift=0;
	while( (1<<shift)<n ) ++shift;
	if( (1<<shift)!=n ) RTEX( "Illegal terrain size" );
	Terrain *t=d_new Terrain( shift );
	return insertEntity( t,p );
}

Entity *  bbLoadTerrain( BBStr *file,Entity *p ){
	if (!debugParent(p,"LoadTerrain")) return 0;
	gxCanvas *c=gx_graphics->loadCanvas( *file,gxCanvas::CANVAS_HIGHCOLOR );
	if( !c ) RTEX( "Unable to load heightmap image" );
	int w=c->getWidth(),h=c->getHeight();
	if( w!=h ) RTEX( "Terrain must be square" );
	int shift=0;
	while( (1<<shift)<w ) ++shift;
	if( (1<<shift)!=w ) RTEX( "Illegal terrain size" );
	Terrain *t=d_new Terrain( shift );
	c->lock();
	for( int y=0;y<h;++y ){
		for( int x=0;x<w;++x ){
			int rgb=c->getPixelFast( x,y );
			int r=(rgb>>16)&0xff,g=(rgb>>8)&0xff,b=rgb&0xff;
			float p=(r>g?(r>b?r:b):(g>b?g:b))/255.0f;
			t->setHeight( x,h-1-y,p,false );
		}
	}
	c->unlock();
	gx_graphics->freeCanvas( c );
	return insertEntity( t,p );
}

void  bbTerrainDetail( Terrain *t,int n,int m ){
	if (!debugTerrain(t,"TerrainDetail")) return;
	t->setDetail( n,!!m );
}

void  bbTerrainShading( Terrain *t,int enable ){
	if (!debugTerrain(t,"TerrainShading")) return;
	t->setShading( !!enable );
}

float  bbTerrainX( Terrain *t,float x,float y,float z ){
	if (!debugTerrain(t,"TerrainX")) return 0;
	return terrainVector( t,x,y,z ).x;
}

float  bbTerrainY( Terrain *t,float x,float y,float z ){
	if (!debugTerrain(t,"TerrainY")) return 0;
	return terrainVector( t,x,y,z ).y;
}

float  bbTerrainZ( Terrain *t,float x,float y,float z ){
	if (!debugTerrain(t,"TerrainZ")) return 0;
	return terrainVector( t,x,y,z ).z;
}

int  bbTerrainSize( Terrain *t ){
	if (!debugTerrain(t,"TerrainSize")) return 0;
	return t->getSize();
}

float  bbTerrainHeight( Terrain *t,int x,int z ){
	if (!debugTerrain(t,"TerrainHeight")) return 0;
	return t->getHeight( x,z );
}

void  bbModifyTerrain( Terrain *t,int x,int z,float h,int realtime ){
	if (!debugTerrain(t,"ModifyTerrain")) return;
	t->setHeight( x,z,h,!!realtime );
}

////////////////////
// AUDIO COMMANDS //
////////////////////
Entity *  bbGetListener( Entity *p,float roll,float dopp,float dist ){
	if (!debugParent(p,"GetListener")) return 0;
	
	if (!listener){
        listener=d_new Listener( roll,dopp,dist );
        insertEntity( listener,0 );
    }
    listener->set(roll,dopp,dist);
    listener->setParent(p);
	return listener;
}

gxChannel *  bbEmitSound( gxSound *sound,Object *o ){
	if (!debugObject(o,"EmitSound")) return 0;
	if( !listener ) bbGetListener( 0,1.f,1.f,1.f );
	return o->emitSound( sound );
}

/////////////////////
// ENTITY COMMANDS //
/////////////////////
Entity *  bbCopyEntity( Entity *e,Entity *p ){
	//if( debug ){
	if (!debugEntity(e,"CopyEntity")) return 0;
	if (!debugParent(p,"CopyEntity")) return 0; 
	//}
	Entity *t=e->getObject()->copy();
	if( !t ) return 0;
	return insertEntity( t,p );
}

void  bbFreeEntity( Entity *e ){
	if( !e ) return;
	if (!debugEntity(e,"FreeEntity")) return;
	//if( debug ){
	erase(e);
	//}
	delete e;
}

void  bbHideEntity( Entity *e ){
	if (!debugEntity(e,"HideEntity")) return;
	e->setEnabled(false);
	e->setVisible(false);
}

void  bbShowEntity( Entity *e ){
	if (!debugEntity(e,"ShowEntity")) return;
	e->setVisible(true);
	e->setEnabled(true);
	e->getObject()->reset();
}

void  bbEntityParent( Entity *e,Entity *p,int global ){
	if (!debugEntity(e,"EntityParent")) return;
	if (!debugParent(p,"EntityParent")) return;
	if( debug ){
		Entity *t=p;
		while( t ){
			if( t==e ){
				RTEX( "Entity cannot be parented to itself!" );
			}
			t=t->getParent();
		}
	} else {
		Entity *t=p;
		while( t ){
			if( t==e ){
				errorLog.push_back(std::string("EntityParent: Entity cannot be parented to itself!"));
				return;
			}
			t=t->getParent();
		}
	}

	if( e->getParent()==p ) return;

	if( global ){
		Transform t=e->getWorldTform();
		e->setParent( p );
		e->setWorldTform( t );
	}else{
		e->setParent( p );
		e->getObject()->reset();
	}
}

int  bbCountChildren( Entity *e ){
	if (!debugEntity(e,"CountChildren")) return 0;
	int n=0;
	for( Entity *p=e->children();p;p=p->successor() ) ++n;
	return n;
}

Entity *  bbGetChild( Entity *e,int index ){
	if (!debugEntity(e,"GetChild")) return 0;
	Entity *p=e->children();
	while( --index && p ) p=p->successor();
	return p;
}

Entity *  bbFindChild( Entity *e,BBStr *t ){
	if (!debugEntity(e,"FindChild")) return 0;
	e=findChild( e,*t );
	delete t;
	return e;
}

////////////////////////
// ANIMATION COMMANDS //
////////////////////////
int  bbLoadAnimSeq( Object *o,BBStr *f ){
	if (!debugObject(o,"LoadAnimSeq")) return 0;
	if( Animator *anim=o->getAnimator() ){
		Entity *t=loadEntity( f->c_str(),MeshLoader::HINT_ANIMONLY );
		delete f;
		if( t ){
			if( Animator *p=t->getObject()->getAnimator() ){
				anim->addSeqs( p );
			}
			delete t;
		}
		return anim->numSeqs()-1;
	}else{
		delete f;
	}
	return -1;
}

void  bbSetAnimTime( Object *o,float time,int seq ){
	if (!debugObject(o,"SetAnimTime")) return;
	if( Animator *anim=o->getAnimator() ){
		anim->setAnimTime( time,seq );
	}else{
		if (debug) {
			RTEX( "Entity has no animation" );
		} else {
			errorLog.push_back("SetAnimTime: Entity has no animation");
		}
	}
}

void  bbAnimate( Object *o,int mode,float speed,int seq,float trans ){
	if (!debugObject(o,"Animate")) return;
	if( Animator *anim=o->getAnimator() ){
		anim->animate( mode,speed,seq,trans );
	}else{
		if (debug) {
			RTEX( "Entity has no animation" );
		} else {
			errorLog.push_back("Animate: Entity has no animation");
		}
	}
}

void  bbSetAnimKey( Object *o,int frame,int pos_key,int rot_key,int scl_key ){
	if (!debugObject(o,"SetAnimKey")) return;
	Animation anim=o->getAnimation();
	if( pos_key ) anim.setPositionKey( frame,o->getLocalPosition() );
	if( rot_key ) anim.setRotationKey( frame,o->getLocalRotation() );
	if( scl_key ) anim.setScaleKey( frame,o->getLocalScale() );
	o->setAnimation( anim );
}

int  bbExtractAnimSeq( Object *o,int first,int last,int seq ){
	if (!debugObject(o,"ExtractAnimSeq")) return 0;
	if( Animator *anim=o->getAnimator() ){
		anim->extractSeq( first,last,seq );
		return anim->numSeqs()-1;
	}
	return -1;
}

int  bbAddAnimSeq( Object *o,int length ){
	if (!debugObject(o,"AddAnimSeq")) return 0;
	Animator *anim=o->getAnimator();
	if( anim ){
		anim->addSeq( length );
	}else{
		anim=d_new Animator( o,length );
		o->setAnimator( anim );
	}
	return anim->numSeqs()-1;
}

int  bbAnimSeq( Object *o ){
	if (!debugObject(o,"AnimSeq")) return 0;
	if( Animator *anim=o->getAnimator() ) return anim->animSeq();
	return -1;
}

float  bbAnimTime( Object *o ){
	if (!debugObject(o,"AnimTime")) return 0;
	if( Animator *anim=o->getAnimator() ) return anim->animTime();
	return -1;
}

int  bbAnimLength( Object *o ){
	if (!debugObject(o,"AnimLength")) return 0;
	if( Animator *anim=o->getAnimator() ) return anim->animLen();
	return -1;
}

int  bbAnimating( Object *o ){
	if (!debugObject(o,"Animating")) return 0;
	if( Animator *anim=o->getAnimator() ) return anim->animating();
	return 0;
}

////////////////////////////////
// ENTITY SPECIAL FX COMMANDS //
////////////////////////////////
void  bbPaintEntity( Model *m,Brush *b ){
	//if( debug ){
	if (!debugModel(m,"PaintEntity")) return;
	if (!debugBrush(b,"PaintEntity")) return;
	//}
	m->setBrush( *b );
}

void  bbEntityColor( Model *m,float r,float g,float b ){
	if (!debugModel(m,"EntityColor")) return;
	m->setColor( Vector( r*ctof,g*ctof,b*ctof ) );
}

void  bbEntityAlpha( Model *m,float alpha ){
	if (!debugModel(m,"EntityAlpha")) return;
	m->setAlpha( alpha );
}

void  bbEntityShininess( Model *m,float shininess ){
	if (!debugModel(m,"EntityShininess")) return;
	m->setShininess( shininess );
}

void  bbEntityTexture( Model *m,Texture *t,int frame,int index ){
	if (!debugModel(m,"EntityTexture")) return;
	if (!debugTexture(t,"EntityTexture")) return;
	m->setTexture( index,*t,frame );
}

void  bbEntityBlend( Model *m,int blend ){
	if (!debugModel(m,"EntityBlend")) return;
	m->setBlend( blend );
}

void  bbEntityFX( Model *m,int fx ){
	if (!debugModel(m,"EntityFX")) return;
	m->setFX( fx );
}

void  bbEntityAutoFade( Model *m,float nr,float fr ){
	if (!debugModel(m,"EntityAutoFade")) return;
	m->setAutoFade( nr,fr );
}

void  bbEntityOrder( Object *o,int n ){
	if( debug ){
		debugEntity(o,"EntityOrder");
		if( !o->getModel() && !o->getCamera() ){
			RTEX( "Entity is not a model or camera" );
		}
	} else {
		if (!debugEntity(o,"EntityOrder")) return;
		if( !o->getModel() && !o->getCamera() ){
			errorLog.push_back(std::string("EntityOrder: Entity is not a model or camera"));
			return;
		}
	}
	o->setOrder( n );
}

//////////////////////////////
// ENTITY PROPERTY COMMANDS //
//////////////////////////////
float  bbEntityX( Entity *e,int global ){
	if (!debugEntity(e,"EntityX")) return 0;
	return global ? e->getWorldPosition().x : e->getLocalPosition().x;
}

float  bbEntityY( Entity *e,int global ){
	if (!debugEntity(e,"EntityY")) return 0;
	return global ? e->getWorldPosition().y : e->getLocalPosition().y;
}

float  bbEntityZ( Entity *e,int global ){
	if (!debugEntity(e,"EntityZ")) return 0;
	return global ? e->getWorldPosition().z : e->getLocalPosition().z;
}

float  bbEntityPitch( Entity *e,int global ){
	if (!debugEntity(e,"EntityPitch")) return 0;
	return quatPitch( global ? e->getWorldRotation() : e->getLocalRotation() ) * rtod;
}

float  bbEntityYaw( Entity *e,int global ){
	if (!debugEntity(e,"EntityYaw")) return 0;
	return quatYaw( global ? e->getWorldRotation() : e->getLocalRotation() ) * rtod;
}

float  bbEntityRoll( Entity *e,int global ){
	if (!debugEntity(e,"EntityRoll")) return 0;
	return quatRoll( global ? e->getWorldRotation() : e->getLocalRotation() ) * rtod;
}

float  bbGetMatElement( Entity *e,int row,int col ){
	if (!debugEntity(e,"GetMatElement")) return 0;
	return row<3 ? e->getWorldTform().m[row][col] : e->getWorldTform().v[col];
}

void  bbTFormPoint( float x,float y,float z,Entity *src,Entity *dest ){
	//if( debug ){
	if( src ) { if (!debugEntity(src,"TFormPoint (src)")) return; }
	if( dest ) { if (!debugEntity(dest,"TFormPoint (dest)")) return; }
	//}
	tformed=Vector( x,y,z );
	if( src ) tformed=src->getWorldTform() * tformed;
	if( dest ) tformed=-dest->getWorldTform() * tformed;
}

void  bbTFormVector( float x,float y,float z,Entity *src,Entity *dest ){
	//if( debug ){
	if( src ) { if (!debugEntity(src,"TFormVector (src)")) return; }
	if( dest ) { if (!debugEntity(dest,"TFormVector (dest)")) return; }
	//}
	tformed=Vector( x,y,z );
	if( src ) tformed=src->getWorldTform().m * tformed;
	if( dest ) tformed=-dest->getWorldTform().m * tformed;
}

void  bbTFormNormal( float x,float y,float z,Entity *src,Entity *dest ){
	//if( debug ){
	if( src ) { if (!debugEntity(src,"TFormNormal (src)")) return; }
	if( dest ) { if (!debugEntity(dest,"TFormNormal (dest)")) return; }
	//}
	tformed=Vector( x,y,z );
	if( src ) tformed=(src->getWorldTform().m).cofactor() * tformed;
	if( dest ) tformed=(-dest->getWorldTform().m).cofactor() * tformed;
	tformed.normalize();
}

float  bbTFormedX(){
	return tformed.x;
}

float  bbTFormedY(){
	return tformed.y;
}

float  bbTFormedZ(){
	return tformed.z;
}

float  bbVectorYaw( float x,float y,float z ){
	return Vector(x,y,z).yaw() * rtod;
}

float  bbVectorPitch( float x,float y,float z ){
	return Vector(x,y,z).pitch() * rtod;
}

float  bbDeltaYaw( Entity *src,Entity *dest ){
	float x=src->getWorldTform().m.k.yaw();
	float y=(dest->getWorldTform().v-src->getWorldTform().v).yaw();
	float d=y-x;
	if( d<-PI ) d+=TWOPI;
	else if( d>=PI ) d-=TWOPI;
	return d*rtod;
}

float  bbDeltaPitch( Entity *src,Entity *dest ){
	float x=src->getWorldTform().m.k.pitch();
	float y=(dest->getWorldTform().v-src->getWorldTform().v).pitch();
	float d=y-x;
	if( d<-PI ) d+=TWOPI;
	else if( d>=PI ) d-=TWOPI;
	return d*rtod;
}

///////////////////////////////
// ENTITY COLLISION COMMANDS //
///////////////////////////////
void  bbResetEntity( Object *o ){
	if (!debugObject(o,"ResetEntity")) return;
	o->reset();
}

static void entityType( Entity *e,int type ){
	e->getObject()->setCollisionType(type);
	e->getObject()->reset();
	for( Entity *p=e->children();p;p=p->successor() ){
		entityType( p,type );
	}
}

void  bbEntityType( Object *o,int type,int recurs ){
	if (!debugObject(o,"EntityType")) return;
	if( debug ){
		if( type<0 || type>999 ) RTEX( "EntityType ID must be in the range 0...999" );
	} else {
		if( type<0 || type>999 ) {
			errorLog.push_back("EntityType: ID must be in the range 0...999");
			return;
		}
	}
	if( recurs ) entityType( o,type );
	else{
		o->setCollisionType(type);
		o->reset();
	}
}

void  bbEntityPickMode( Object *o,int mode,int obs ){
	if (!debugObject(o,"EntityPickMode")) return;
	o->setPickGeometry( mode );
	o->setObscurer( !!obs );
}

Entity *  bbGetParent( Entity *e ){
	if (!debugEntity(e,"GetParent")) return 0;
	return e->getParent();
}

int  bbGetEntityType( Object *o ){
	if (!debugObject(o,"GetEntityType")) return 0;
	return o->getCollisionType();
}

void  bbEntityRadius( Object *o,float x_radius,float y_radius ){
	if (!debugObject(o,"EntityRadius")) return;
	Vector radii( x_radius,y_radius ? y_radius : x_radius,x_radius );
	o->setCollisionRadii( radii );
}

void  bbEntityBox( Object *o,float x,float y,float z,float w,float h,float d ){
	if (!debugObject(o,"EntityBox")) return;
	Box b( Vector(x,y,z) );
	b.update( Vector( x+w,y+h,z+d ) );
	o->setCollisionBox( b );
}

Object *  bbEntityCollided( Object *o,int type ){
	if (!debugObject(o,"EntityCollided")) return 0;
	Object::Collisions::const_iterator it;
	const Object::Collisions &c=o->getCollisions();
	for( it=c.begin();it!=c.end();++it ){
		const ObjCollision *c=*it;
		if( c->with->getCollisionType()==type ) return c->with;
	}
	return 0;
}

int  bbCountCollisions( Object *o ){
	if (!debugObject(o,"CountCollisions")) return 0;
	return o->getCollisions().size();
}

float  bbCollisionX( Object *o,int index ){
	if (!debugColl(o,index,"CollisionX")) return 0;
	return o->getCollisions()[index-1]->coords.x;
}

float  bbCollisionY( Object *o,int index ){
	if (!debugColl(o,index,"CollisionY")) return 0;
	return o->getCollisions()[index-1]->coords.y;
}

float  bbCollisionZ( Object *o,int index ){
	if (!debugColl(o,index,"CollisionZ")) return 0;
	return o->getCollisions()[index-1]->coords.z;
}

float  bbCollisionNX( Object *o,int index ){
	if (!debugColl(o,index,"CollisionNX")) return 0;
	return o->getCollisions()[index-1]->collision.normal.x;
}

float  bbCollisionNY( Object *o,int index ){
	if (!debugColl(o,index,"CollisionNY")) return 0;
	return o->getCollisions()[index-1]->collision.normal.y;
}

float  bbCollisionNZ( Object *o,int index ){
	if (!debugColl(o,index,"CollisionNZ")) return 0;
	return o->getCollisions()[index-1]->collision.normal.z;
}

float  bbCollisionTime( Object *o,int index ){
	if (!debugColl(o,index,"CollisionTime")) return 0;
	return o->getCollisions()[index-1]->collision.time;
}

Object *  bbCollisionEntity( Object *o,int index ){
	if (!debugColl(o,index,"CollisionEntity")) return 0;
	return o->getCollisions()[index-1]->with;
}

void *  bbCollisionSurface( Object *o,int index ){
	if (!debugColl(o,index,"CollisionSurface")) return 0;
	return o->getCollisions()[index-1]->collision.surface;
}

int  bbCollisionTriangle( Object *o,int index ){
	if (!debugColl(o,index,"CollisionTriangle")) return 0;
	return o->getCollisions()[index-1]->collision.index;
}

float  bbEntityDistance( Entity *src,Entity *dest ){
	if (!debugEntity(src,"EntityDistance (src)")) return 0;
	if (!debugEntity(dest,"EntityDistance (dest)")) return 0;
	return src->getWorldPosition().distance( dest->getWorldPosition() );
}

////////////////////////////////////
// ENTITY TRANSFORMATION COMMANDS //
////////////////////////////////////
void  bbMoveEntity( Entity *e,float x,float y,float z ){
	if (!debugEntity(e,"MoveEntity")) return;
	e->setLocalPosition( e->getLocalPosition()+e->getLocalRotation()*Vector(x,y,z) );
}

void  bbTurnEntity( Entity *e,float p,float y,float r,int global ){
	if (!debugEntity(e,"TurnEntity")) return;
	global?
	e->setWorldRotation( rotationQuat( p*dtor,y*dtor,r*dtor )*e->getWorldRotation() ):
	e->setLocalRotation( e->getLocalRotation()*rotationQuat( p*dtor,y*dtor,r*dtor ) );
}

void  bbTranslateEntity( Entity *e,float x,float y,float z,int global ){
	if (!debugEntity(e,"TranslateEntity")) return;
	global?
	e->setWorldPosition( e->getWorldPosition()+Vector( x,y,z ) ):
	e->setLocalPosition( e->getLocalPosition()+Vector( x,y,z ) );
}

void  bbPositionEntity( Entity *e,float x,float y,float z,int global ){
	if (!debugEntity(e,"PositionEntity")) return;
	global?
	e->setWorldPosition(Vector(x,y,z)):
	e->setLocalPosition(Vector(x,y,z));
}

void  bbScaleEntity( Entity *e,float x,float y,float z,int global ){
	if (!debugEntity(e,"ScaleEntity")) return;
	global?
	e->setWorldScale(Vector(x,y,z)):
	e->setLocalScale(Vector(x,y,z));
}

void  bbRotateEntity( Entity *e,float p,float y,float r,int global ){
	if (!debugEntity(e,"RotateEntity")) return;
	global?
	e->setWorldRotation( rotationQuat( p*dtor,y*dtor,r*dtor ) ):
	e->setLocalRotation( rotationQuat( p*dtor,y*dtor,r*dtor ) );
}

void  bbPointEntity( Entity *e,Entity *t,float roll ){
	//if( debug ){ debugEntity(e);debugEntity(t); }
	if (!debugEntity(e,"PointEntity (src)")) return;
	if (!debugEntity(t,"PointEntity (dest)")) return;
	Vector v=t->getWorldTform().v-e->getWorldTform().v;
	e->setWorldRotation( rotationQuat( v.pitch(),v.yaw(),roll*dtor ) );
}

void  bbAlignToVector( Entity *e,float nx,float ny,float nz,int axis,float rate ){
	Vector ax( nx,ny,nz );
	float l=ax.length();
	if( l<=EPSILON ) return;
	ax/=l;

	Quat q=e->getWorldRotation();
	Vector tv=(axis==1) ? q.i() : (axis==2 ? q.j() : q.k());

	float dp=ax.dot( tv );

	if( dp>=1-EPSILON ) return;

	if( dp<=-1+EPSILON ){
		float an=PI*rate/2;
		Vector cp=(axis==1) ? q.j() : (axis==2 ? q.k() : q.i());
		e->setWorldRotation( Quat( cosf(an),cp*sinf(an) ) * q );
		return;
	}

	float an=acosf( dp )*rate/2;
	Vector cp=ax.cross( tv ).normalized();
	e->setWorldRotation( Quat( cosf(an),cp*sinf(an) ) * q );
}

//////////////////////////
// ENTITY MISC COMMANDS //
//////////////////////////
void  bbNameEntity( Entity *e,BBStr *t ){
	if (!debugEntity(e,"NameEntity")) return;
	e->setName( *t );
	delete t;
}

BBStr *  bbEntityName( Entity *e ){
	if (!debugEntity(e,"EntityName")) return d_new BBStr("");
	return d_new BBStr( e->getName() );
}

BBStr *bbEntityClass( Entity *e ){
	if (!debugEntity(e,"EntityClass")) return d_new BBStr("");
	const char *p="Pivot";
	if( e->getLight() ) p="Light";
	else if( e->getCamera() ) p="Camera";
	else if( e->getMirror() ) p="Mirror";
	else if( e->getListener() ) p="Listener";
	else if( Model *t=e->getModel() ){
		if( t->getSprite() ) p="Sprite";
		else if( t->getTerrain() ) p="Terrain";
		else if( t->getPlaneModel() ) p="Plane";
		else if( t->getMeshModel() ) p="Mesh";
		else if( t->getMD2Model() ) p="MD2";
		else if( t->getBSPModel() ) p="BSP";
	}
	return new BBStr(p);
}

void  bbClearWorld( int e,int b,int t ){
	if( e ){
		while( Entity::orphans() ) bbFreeEntity( Entity::orphans() );
	}
	if( b ){
		while( brush_set.size() ) bbFreeBrush( *brush_set.begin() );
	}
	if( t ){
		while( texture_set.size() ) bbFreeTexture( *texture_set.begin() );
	}
}

extern int active_texs;

int  bbActiveTextures(){
	return active_texs;
}

void blitz3d_open(){
	gx_scene=gx_graphics->createScene( 0 );
	if( !gx_scene ) RTEX( "Unable to create 3D Scene" );
	world=d_new World();
	projected=Vector();
	picked.collision=Collision();
	picked.with=0;picked.coords=Vector();
	Texture::clearFilters();
	Texture::addFilter( "",gxCanvas::CANVAS_TEX_RGB|gxCanvas::CANVAS_TEX_MIPMAP );
	loader_mat_map.clear();
	loader_mat_map["x"]=Transform();
	loader_mat_map["3ds"]=Transform(Matrix(Vector(1,0,0),Vector(0,0,1),Vector(0,1,0)));
	listener=0;
	stats_mode=false;
}

void blitz3d_close(){
	if( !gx_scene ) return;
	bbClearWorld( 1,1,1 );
	Texture::clearFilters();
	loader_mat_map.clear();
	delete world;
	gx_graphics->freeScene( gx_scene );
	gx_scene=0;
}

bool blitz3d_create(){
	tri_count=0;
	gx_scene=0;world=0;
	return true;
}

bool blitz3d_destroy(){
	blitz3d_close();
	return true;
}

void blitz3d_link( void (*rtSym)( const char *sym,void *pc ) ){
	rtSym( "LoaderMatrix$file_ext#xx#xy#xz#yx#yy#yz#zx#zy#zz",bbLoaderMatrix );
	rtSym( "HWMultiTex%enable",bbHWMultiTex );
	rtSym( "%HWTexUnits",bbHWTexUnits );
	rtSym( "%GfxDriverCaps3D",bbGfxDriverCaps3D );
	rtSym( "WBuffer%enable",bbWBuffer );
	rtSym( "Dither%enable",bbDither );
	rtSym( "AntiAlias%enable",bbAntiAlias );
	rtSym( "WireFrame%enable",bbWireFrame );
	rtSym( "AmbientLight#red#green#blue",bbAmbientLight );
	rtSym( "ClearCollisions",bbClearCollisions );
	rtSym( "Collisions%source_type%destination_type%method%response",bbCollisions );
	rtSym( "UpdateWorld#elapsed_time=1",bbUpdateWorld );
	rtSym( "CaptureWorld",bbCaptureWorld );
	rtSym( "RenderWorld#tween=1",bbRenderWorld );
	rtSym( "ClearWorld%entities=1%brushes=1%textures=1",bbClearWorld );
	rtSym( "%ActiveTextures",bbActiveTextures );
	rtSym( "%TrisRendered",bbTrisRendered );
	rtSym( "#Stats3D%type",bbStats3D );

	rtSym( "(BBTexture)CreateTexture%width%height%flags=0%frames=1",bbCreateTexture );
	rtSym( "(BBTexture)LoadTexture$file%flags=1",bbLoadTexture );
	rtSym( "(BBTexture)LoadAnimTexture$file%flags%width%height%first%count",bbLoadAnimTexture );
	rtSym( "FreeTexture(BBTexture)texture",bbFreeTexture );
	rtSym( "TextureBlend(BBTexture)texture%blend",bbTextureBlend );
	rtSym( "TextureCoords(BBTexture)texture%coords",bbTextureCoords );
	rtSym( "TextureBumpEnvMat(BBTexture)texture%x%y#envmat",bbTextureBumpEnvMat );
	rtSym( "TextureBumpEnvScale(BBTexture)texture#envmat",bbTextureBumpEnvScale );
	rtSym( "TextureBumpEnvOffset(BBTexture)texture#envoffset",bbTextureBumpEnvOffset );
	rtSym( "ScaleTexture(BBTexture)texture#u_scale#v_scale",bbScaleTexture );
	rtSym( "RotateTexture(BBTexture)texture#angle",bbRotateTexture );
	rtSym( "PositionTexture(BBTexture)texture#u_offset#v_offset",bbPositionTexture );
	rtSym( "TextureLodBias#bias",bbTextureLodBias );
	rtSym( "%TextureWidth(BBTexture)texture",bbTextureWidth );
	rtSym( "%TextureHeight(BBTexture)texture",bbTextureHeight );
	rtSym( "$TextureName(BBTexture)texture",bbTextureName );
	rtSym( "SetCubeFace(BBTexture)texture%face",bbSetCubeFace );
	rtSym( "SetCubeMode(BBTexture)texture%mode",bbSetCubeMode );
	rtSym( "(BBBuffer)TextureBuffer(BBTexture)texture%frame=0",bbTextureBuffer );
	rtSym( "ClearTextureFilters",bbClearTextureFilters );
	rtSym( "TextureFilter$match_text%texture_flags=0",bbTextureFilter );

	rtSym( "(BBBrush)CreateBrush#red=255#green=255#blue=255",bbCreateBrush );
	rtSym( "(BBBrush)LoadBrush$file%texture_flags=1#u_scale=1#v_scale=1",bbLoadBrush );
	rtSym( "FreeBrush(BBBrush)brush",bbFreeBrush );
	rtSym( "BrushColor(BBBrush)brush#red#green#blue",bbBrushColor );
	rtSym( "BrushAlpha(BBBrush)brush#alpha",bbBrushAlpha );
	rtSym( "BrushShininess(BBBrush)brush#shininess",bbBrushShininess );
	rtSym( "BrushTexture(BBBrush)brush(BBTexture)texture%frame=0%index=0",bbBrushTexture );
	rtSym( "(BBTexture)GetBrushTexture(BBBrush)brush%index=0",bbGetBrushTexture );
	rtSym( "BrushBlend(BBBrush)brush%blend",bbBrushBlend );
	rtSym( "BrushFX(BBBrush)brush%fx",bbBrushFX );

	rtSym( "(BBEntity)LoadMesh$file(BBEntity)parent=Null",bbLoadMesh );
	rtSym( "(BBEntity)LoadAnimMesh$file(BBEntity)parent=Null",bbLoadAnimMesh );
	rtSym( "%LoadAnimSeq(BBEntity)entity$file",bbLoadAnimSeq );

	rtSym( "(BBEntity)CreateMesh(BBEntity)parent=Null",bbCreateMesh );
	rtSym( "(BBEntity)CreateCube(BBEntity)parent=Null",bbCreateCube );
	rtSym( "(BBEntity)CreateSphere%segments=8(BBEntity)parent=Null",bbCreateSphere );
	rtSym( "(BBEntity)CreateCylinder%segments=8%solid=1(BBEntity)parent=Null",bbCreateCylinder );
	rtSym( "(BBEntity)CreateCone%segments=8%solid=1(BBEntity)parent=Null",bbCreateCone );
	rtSym( "(BBEntity)CopyMesh(BBEntity)mesh(BBEntity)parent=Null",bbCopyMesh );
	rtSym( "ScaleMesh(BBEntity)mesh#x_scale#y_scale#z_scale",bbScaleMesh );
	rtSym( "RotateMesh(BBEntity)mesh#pitch#yaw#roll",bbRotateMesh );
	rtSym( "PositionMesh(BBEntity)mesh#x#y#z",bbPositionMesh );
	rtSym( "FitMesh(BBEntity)mesh#x#y#z#width#height#depth%uniform=0",bbFitMesh );
	rtSym( "FlipMesh(BBEntity)mesh",bbFlipMesh );
	rtSym( "PaintMesh(BBEntity)mesh(BBBrush)brush",bbPaintMesh );
	rtSym( "AddMesh(BBEntity)source_mesh(BBEntity)dest_mesh",bbAddMesh );
	rtSym( "UpdateNormals(BBEntity)mesh",bbUpdateNormals );
	rtSym( "LightMesh(BBEntity)mesh#red#green#blue#range=0#x=0#y=0#z=0",bbLightMesh );
	rtSym( "#MeshWidth(BBEntity)mesh",bbMeshWidth );
	rtSym( "#MeshHeight(BBEntity)mesh",bbMeshHeight );
	rtSym( "#MeshDepth(BBEntity)mesh",bbMeshDepth );
	rtSym( "(BBEntity)MeshesIntersect(BBEntity)mesh_a(BBEntity)mesh_b",bbMeshesIntersect );
	rtSym( "%CountSurfaces(BBEntity)mesh",bbCountSurfaces );
	rtSym( "(BBSurface)GetSurface(BBEntity)mesh%surface_index",bbGetSurface );
	rtSym( "MeshCullBox(BBEntity)mesh#x#y#z#width#height#depth",bbMeshCullBox );
	
	rtSym( "(BBSurface)CreateSurface(BBEntity)mesh(BBBrush)brush=Null",bbCreateSurface );
	rtSym( "(BBBrush)GetSurfaceBrush(BBSurface)surface",bbGetSurfaceBrush );
	rtSym( "(BBBrush)GetEntityBrush(BBEntity)entity",bbGetEntityBrush );
	rtSym( "(BBSurface)FindSurface(BBEntity)mesh(BBBrush)brush",bbFindSurface );
	rtSym( "ClearSurface(BBSurface)surface%clear_vertices=1%clear_triangles=1",bbClearSurface );
	rtSym( "PaintSurface(BBSurface)surface(BBBrush)brush",bbPaintSurface );
	rtSym( "%AddVertex(BBSurface)surface#x#y#z#u=0#v=0#w=1",bbAddVertex );
	rtSym( "%AddTriangle(BBSurface)surface%v0%v1%v2",bbAddTriangle );
	rtSym( "VertexCoords(BBSurface)surface%index#x#y#z",bbVertexCoords );
	rtSym( "VertexNormal(BBSurface)surface%index#nx#ny#nz",bbVertexNormal );
	rtSym( "VertexColor(BBSurface)surface%index#red#green#blue#alpha=1",bbVertexColor );
	rtSym( "VertexTexCoords(BBSurface)surface%index#u#v#w=1%coord_set=0",bbVertexTexCoords );
	rtSym( "%CountVertices(BBSurface)surface",bbCountVertices );
	rtSym( "%CountTriangles(BBSurface)surface",bbCountTriangles );
	rtSym( "#VertexX(BBSurface)surface%index",bbVertexX );
	rtSym( "#VertexY(BBSurface)surface%index",bbVertexY );
	rtSym( "#VertexZ(BBSurface)surface%index",bbVertexZ );
	rtSym( "#VertexNX(BBSurface)surface%index",bbVertexNX );
	rtSym( "#VertexNY(BBSurface)surface%index",bbVertexNY );
	rtSym( "#VertexNZ(BBSurface)surface%index",bbVertexNZ );
	rtSym( "#VertexRed(BBSurface)surface%index",bbVertexRed );
	rtSym( "#VertexGreen(BBSurface)surface%index",bbVertexGreen );
	rtSym( "#VertexBlue(BBSurface)surface%index",bbVertexBlue );
	rtSym( "#VertexAlpha(BBSurface)surface%index",bbVertexAlpha );
	rtSym( "#VertexU(BBSurface)surface%index%coord_set=0",bbVertexU );
	rtSym( "#VertexV(BBSurface)surface%index%coord_set=0",bbVertexV );
	rtSym( "#VertexW(BBSurface)surface%index%coord_set=0",bbVertexW );
	rtSym( "%TriangleVertex(BBSurface)surface%index%vertex",bbTriangleVertex );

	rtSym( "(BBEntity)CreateCamera(BBEntity)parent=Null",bbCreateCamera );
	rtSym( "CameraZoom(BBEntity)camera#zoom",bbCameraZoom );
	rtSym( "CameraRange(BBEntity)camera#near#far",bbCameraRange );
	rtSym( "CameraClsColor(BBEntity)camera#red#green#blue",bbCameraClsColor );
	rtSym( "CameraClsMode(BBEntity)camera%cls_color%cls_zbuffer",bbCameraClsMode );
	rtSym( "CameraProjMode(BBEntity)camera%mode",bbCameraProjMode );
	rtSym( "CameraViewport(BBEntity)camera%x%y%width%height",bbCameraViewport );
	rtSym( "CameraFogColor(BBEntity)camera#red#green#blue",bbCameraFogColor );
	rtSym( "CameraFogRange(BBEntity)camera#near#far",bbCameraFogRange );
	rtSym( "CameraFogMode(BBEntity)camera%mode",bbCameraFogMode );
	rtSym( "CameraProject(BBEntity)camera#x#y#z",bbCameraProject );
	rtSym( "#ProjectedX",bbProjectedX );
	rtSym( "#ProjectedY",bbProjectedY );
	rtSym( "#ProjectedZ",bbProjectedZ );

	rtSym( "%EntityInView(BBEntity)entity(BBEntity)camera",bbEntityInView );
	rtSym( "%EntityVisible(BBEntity)src_entity(BBEntity)dest_entity",bbEntityVisible );

	rtSym( "(BBEntity)EntityPick(BBEntity)entity#range",bbEntityPick );
	rtSym( "(BBEntity)LinePick#x#y#z#dx#dy#dz#radius=0",bbLinePick );
	rtSym( "(BBEntity)CameraPick(BBEntity)camera#viewport_x#viewport_y",bbCameraPick );

	rtSym( "#PickedX",bbPickedX );
	rtSym( "#PickedY",bbPickedY );
	rtSym( "#PickedZ",bbPickedZ );
	rtSym( "#PickedNX",bbPickedNX );
	rtSym( "#PickedNY",bbPickedNY );
	rtSym( "#PickedNZ",bbPickedNZ );
	rtSym( "#PickedTime",bbPickedTime );
	rtSym( "(BBEntity)PickedEntity",bbPickedEntity );
	rtSym( "(BBSurface)PickedSurface",bbPickedSurface );
	rtSym( "%PickedTriangle",bbPickedTriangle );

	rtSym( "(BBEntity)CreateLight%type=1(BBEntity)parent=Null",bbCreateLight );
	rtSym( "LightColor(BBEntity)light#red#green#blue",bbLightColor );
	rtSym( "LightRange(BBEntity)light#range",bbLightRange );
	rtSym( "LightConeAngles(BBEntity)light#inner_angle#outer_angle",bbLightConeAngles );

	rtSym( "(BBEntity)CreatePivot(BBEntity)parent=Null",bbCreatePivot );

	rtSym( "(BBEntity)CreateSprite(BBEntity)parent=Null",bbCreateSprite );
	rtSym( "(BBEntity)LoadSprite$file%texture_flags=1(BBEntity)parent=Null",bbLoadSprite );
	rtSym( "RotateSprite(BBEntity)sprite#angle",bbRotateSprite );
	rtSym( "ScaleSprite(BBEntity)sprite#x_scale#y_scale",bbScaleSprite );
	rtSym( "HandleSprite(BBEntity)sprite#x_handle#y_handle",bbHandleSprite );
	rtSym( "SpriteViewMode(BBEntity)sprite%view_mode",bbSpriteViewMode );

	rtSym( "(BBEntity)LoadMD2$file(BBEntity)parent=Null",bbLoadMD2 );
	rtSym( "AnimateMD2(BBEntity)md2%mode=1#speed=1%first_frame=0%last_frame=9999#transition=0",bbAnimateMD2 );
	rtSym( "#MD2AnimTime(BBEntity)md2",bbMD2AnimTime );
	rtSym( "%MD2AnimLength(BBEntity)md2",bbMD2AnimLength );
	rtSym( "%MD2Animating(BBEntity)md2",bbMD2Animating );

	rtSym( "(BBEntity)LoadBSP$file#gamma_adj=0(BBEntity)parent=Null",bbLoadBSP );
	rtSym( "BSPLighting(BBEntity)bsp%use_lightmaps",bbBSPLighting );
	rtSym( "BSPAmbientLight(BBEntity)bsp#red#green#blue",bbBSPAmbientLight );

	rtSym( "(BBEntity)CreateMirror(BBEntity)parent=Null",bbCreateMirror );

	rtSym( "(BBEntity)CreatePlane%segments=1(BBEntity)parent=Null",bbCreatePlane );

	rtSym( "(BBEntity)CreateTerrain%grid_size(BBEntity)parent=Null",bbCreateTerrain );
	rtSym( "(BBEntity)LoadTerrain$heightmap_file(BBEntity)parent=Null",bbLoadTerrain );
	rtSym( "TerrainDetail(BBEntity)terrain%detail_level%morph=0",bbTerrainDetail );
	rtSym( "TerrainShading(BBEntity)terrain%enable",bbTerrainShading );
	rtSym( "#TerrainX(BBEntity)terrain#world_x#world_y#world_z",bbTerrainX );
	rtSym( "#TerrainY(BBEntity)terrain#world_x#world_y#world_z",bbTerrainY );
	rtSym( "#TerrainZ(BBEntity)terrain#world_x#world_y#world_z",bbTerrainZ );
	rtSym( "%TerrainSize(BBEntity)terrain",bbTerrainSize );
	rtSym( "#TerrainHeight(BBEntity)terrain%terrain_x%terrain_z",bbTerrainHeight );
	rtSym( "ModifyTerrain(BBEntity)terrain%terrain_x%terrain_z#height%realtime=0",bbModifyTerrain );

	rtSym( "(BBEntity)GetListener(BBEntity)parent=Null#rolloff_factor=1#doppler_scale=1#distance_scale=1",bbGetListener );
	rtSym( "(BBChannel)EmitSound(BBSound)sound(BBEntity)entity",bbEmitSound );

	rtSym( "(BBEntity)CopyEntity(BBEntity)entity(BBEntity)parent=Null",bbCopyEntity );

	rtSym( "#EntityX(BBEntity)entity%global=0",bbEntityX );
	rtSym( "#EntityY(BBEntity)entity%global=0",bbEntityY );
	rtSym( "#EntityZ(BBEntity)entity%global=0",bbEntityZ );
	rtSym( "#EntityPitch(BBEntity)entity%global=0",bbEntityPitch );
	rtSym( "#EntityYaw(BBEntity)entity%global=0",bbEntityYaw );
	rtSym( "#EntityRoll(BBEntity)entity%global=0",bbEntityRoll );
	rtSym( "#GetMatElement(BBEntity)entity%row%column",bbGetMatElement );
	rtSym( "TFormPoint#x#y#z(BBEntity)source_entity(BBEntity)dest_entity",bbTFormPoint );
	rtSym( "TFormVector#x#y#z(BBEntity)source_entity(BBEntity)dest_entity",bbTFormVector );
	rtSym( "TFormNormal#x#y#z(BBEntity)source_entity(BBEntity)dest_entity",bbTFormNormal );
	rtSym( "#TFormedX",bbTFormedX );
	rtSym( "#TFormedY",bbTFormedY );
	rtSym( "#TFormedZ",bbTFormedZ );
	rtSym( "#VectorYaw#x#y#z",bbVectorYaw );
	rtSym( "#VectorPitch#x#y#z",bbVectorPitch );
	rtSym( "#DeltaPitch(BBEntity)src_entity(BBEntity)dest_entity",bbDeltaPitch );
	rtSym( "#DeltaYaw(BBEntity)src_entity(BBEntity)dest_entity",bbDeltaYaw );

	rtSym( "ResetEntity(BBEntity)entity",bbResetEntity );
	rtSym( "EntityType(BBEntity)entity%collision_type%recursive=0",bbEntityType );
	rtSym( "EntityPickMode(BBEntity)entity%pick_geometry%obscurer=1",bbEntityPickMode );
	rtSym( "(BBEntity)GetParent(BBEntity)entity",bbGetParent );
	rtSym( "%GetEntityType(BBEntity)entity",bbGetEntityType );
	rtSym( "EntityRadius(BBEntity)entity#x_radius#y_radius=0",bbEntityRadius );
	rtSym( "EntityBox(BBEntity)entity#x#y#z#width#height#depth",bbEntityBox );
	rtSym( "#EntityDistance(BBEntity)source_entity(BBEntity)destination_entity",bbEntityDistance );
	rtSym( "(BBEntity)EntityCollided(BBEntity)entity%type",bbEntityCollided );

	rtSym( "%CountCollisions(BBEntity)entity",bbCountCollisions );
	rtSym( "#CollisionX(BBEntity)entity%collision_index",bbCollisionX );
	rtSym( "#CollisionY(BBEntity)entity%collision_index",bbCollisionY );
	rtSym( "#CollisionZ(BBEntity)entity%collision_index",bbCollisionZ );
	rtSym( "#CollisionNX(BBEntity)entity%collision_index",bbCollisionNX );
	rtSym( "#CollisionNY(BBEntity)entity%collision_index",bbCollisionNY );
	rtSym( "#CollisionNZ(BBEntity)entity%collision_index",bbCollisionNZ );
	rtSym( "#CollisionTime(BBEntity)entity%collision_index",bbCollisionTime );
	rtSym( "(BBEntity)CollisionEntity(BBEntity)entity%collision_index",bbCollisionEntity );
	rtSym( "(BBSurface)CollisionSurface(BBEntity)entity%collision_index",bbCollisionSurface );
	rtSym( "%CollisionTriangle(BBEntity)entity%collision_index",bbCollisionTriangle );

	rtSym( "MoveEntity(BBEntity)entity#x#y#z",bbMoveEntity );
	rtSym( "TurnEntity(BBEntity)entity#pitch#yaw#roll%global=0",bbTurnEntity );
	rtSym( "TranslateEntity(BBEntity)entity#x#y#z%global=0",bbTranslateEntity );
	rtSym( "PositionEntity(BBEntity)entity#x#y#z%global=0",bbPositionEntity );
	rtSym( "ScaleEntity(BBEntity)entity#x_scale#y_scale#z_scale%global=0",bbScaleEntity );
	rtSym( "RotateEntity(BBEntity)entity#pitch#yaw#roll%global=0",bbRotateEntity );
	rtSym( "PointEntity(BBEntity)entity(BBEntity)target#roll=0",bbPointEntity );
	rtSym( "AlignToVector(BBEntity)entity#vector_x#vector_y#vector_z%axis#rate=1",bbAlignToVector );
	rtSym( "SetAnimTime(BBEntity)entity#time%anim_seq=0",bbSetAnimTime );
	rtSym( "Animate(BBEntity)entity%mode=1#speed=1%sequence=0#transition=0",bbAnimate );
	rtSym( "SetAnimKey(BBEntity)entity%frame%pos_key=1%rot_key=1%scale_key=1",bbSetAnimKey );
	rtSym( "%AddAnimSeq(BBEntity)entity%length",bbAddAnimSeq );
	rtSym( "%ExtractAnimSeq(BBEntity)entity%first_frame%last_frame%anim_seq=0",bbExtractAnimSeq );
	rtSym( "%AnimSeq(BBEntity)entity",bbAnimSeq );
	rtSym( "#AnimTime(BBEntity)entity",bbAnimTime );
	rtSym( "%AnimLength(BBEntity)entity",bbAnimLength );
	rtSym( "%Animating(BBEntity)entity",bbAnimating );

	rtSym( "EntityParent(BBEntity)entity(BBEntity)parent%global=1",bbEntityParent );
	rtSym( "%CountChildren(BBEntity)entity",bbCountChildren );
	rtSym( "(BBEntity)GetChild(BBEntity)entity%index",bbGetChild );
	rtSym( "(BBEntity)FindChild(BBEntity)entity$name",bbFindChild );

	rtSym( "PaintEntity(BBEntity)entity(BBBrush)brush",bbPaintEntity );
	rtSym( "EntityColor(BBEntity)entity#red#green#blue",bbEntityColor );
	rtSym( "EntityAlpha(BBEntity)entity#alpha",bbEntityAlpha );
	rtSym( "EntityShininess(BBEntity)entity#shininess",bbEntityShininess );
	rtSym( "EntityTexture(BBEntity)entity(BBTexture)texture%frame=0%index=0",bbEntityTexture );
	rtSym( "EntityBlend(BBEntity)entity%blend",bbEntityBlend );
	rtSym( "EntityFX(BBEntity)entity%fx",bbEntityFX );
	rtSym( "EntityAutoFade(BBEntity)entity#near#far",bbEntityAutoFade );
	rtSym( "EntityOrder(BBEntity)entity%order",bbEntityOrder );
	rtSym( "HideEntity(BBEntity)entity",bbHideEntity );
	rtSym( "ShowEntity(BBEntity)entity",bbShowEntity );
	rtSym( "FreeEntity(BBEntity)entity",bbFreeEntity );

	rtSym( "NameEntity(BBEntity)entity$name",bbNameEntity );
	rtSym( "$EntityName(BBEntity)entity",bbEntityName );
	rtSym( "$EntityClass(BBEntity)entity",bbEntityClass );
}

#endif
