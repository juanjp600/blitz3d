#ifndef GXRUNTIME_H
#define GXRUNTIME_H

#include <windows.h>
#include <string>
#include <vector>

class gxAudio;
class gxInput;
class gxGraphics;
class gxFileSystem;
class gxTimer;

class gxRuntime{
public:
	HWND hwnd;
	HINSTANCE hinst;

	gxAudio *audio;
	gxInput *input;
	gxGraphics *graphics;
	gxFileSystem *fileSystem;

	void flip( bool vwait );
	void moveMouse( int x,int y );

	LRESULT windowProc( HWND hwnd,UINT msg,WPARAM w,LPARAM l );

	struct GfxMode;	

	//return true if program should continue, or false for quit.
	bool idle();
	bool delay( int ms );

	bool execute( const std::string &cmd );
	void setTitle( const std::string &title,const std::string &close );
	int  getMilliSecs();
	void setPointerVisible( bool vis );

	std::string commandLine();

	std::string systemProperty( const std::string &t );

	void debugStop();
	//void debugProfile( int per );
	void debugStmt( int pos,const char *file );
	void debugEnter( void *frame,void *env,const char *func );
	void debugLeave();
	void debugInfo( const char *t );
	void debugError( const char *t );
	void debugLog( const char *t );

	gxAudio *openAudio( int flags );
	void closeAudio( gxAudio *audio );

	gxInput *openInput( int flags );
	void closeInput( gxInput *input );

	gxGraphics *openGraphics( int w,int h,int d,int driver,int flags );
	void closeGraphics( gxGraphics *graphics );
	bool graphicsLost();

	gxFileSystem *openFileSystem( int flags );
	void closeFileSystem( gxFileSystem *filesys );

	gxTimer *createTimer( int hertz );
	void freeTimer( gxTimer *timer );

	int numGraphicsModes( int driver );
	void graphicsModeInfo( int driver,int mode,int *w,int *h,int *d );
	/***** APP INTERFACE *****/
public:
	static gxRuntime *openRuntime( HINSTANCE hinst,const std::string &cmd_line,class Debugger *debugger );
	static void closeRuntime( gxRuntime *runtime );

	void asyncStop();
	void asyncRun();
	void asyncEnd();
};

#endif