
#include "std.h"
#include "gxruntime.h"

#include "../debugger/debugger.h"

//#include "zmouse.h"

//#define SPI_SETMOUSESPEED	113

static string app_title;
static string app_close;
static gxRuntime *runtime;
static bool busy,suspended;
static volatile bool run_flag;

static Debugger *debugger;

void gxRuntime::flip(bool vwait) {

}

void gxRuntime::moveMouse(int x, int y) {

}

bool gxRuntime::idle() {
	return false;
}

bool gxRuntime::delay(int ms) {
	return true;
}

bool gxRuntime::execute(const std::string &cmd) {
	return true;
}

void gxRuntime::setTitle(const std::string &title, const std::string &close) {
	
}

int gxRuntime::getMilliSecs() {
	return 0;
}

void gxRuntime::setPointerVisible(bool vis) {

}

std::string gxRuntime::commandLine() {
	return "";
}

std::string gxRuntime::systemProperty(const std::string &t) {
	return "";
}

void gxRuntime::forceSuspend() {

}

void gxRuntime::debugStmt( int pos,const char *file ){
	if( debugger ) debugger->debugStmt( pos,file );
}

void gxRuntime::debugStop(){
	if( !suspended ) forceSuspend();
}

void gxRuntime::debugEnter( void *frame,void *env,const char *func ){
	if( debugger ) debugger->debugEnter( frame,env,func );
}

void gxRuntime::debugLeave(){
	if( debugger ) debugger->debugLeave();
}

void gxRuntime::debugError( const char *t ){
	if( !debugger ) return;
	Debugger *d=debugger;
	asyncEnd();
	if( !suspended ){
		forceSuspend();
	}
	d->debugMsg( t,true );
}

void gxRuntime::debugInfo( const char *t ){
	if( !debugger ) return;
	Debugger *d=debugger;
	asyncEnd();
	if( !suspended ){
		forceSuspend();
	}
	d->debugMsg( t,false );
}

void gxRuntime::debugLog( const char *t ){
	if( debugger ) debugger->debugLog( t );
}

gxAudio *gxRuntime::openAudio(int flags) {
	return 0;
}
void gxRuntime::closeAudio(gxAudio *audio) {

}

gxInput *gxRuntime::openInput(int flags) {
	return 0;
}
void gxRuntime::closeInput(gxInput *input) {

}

gxGraphics *gxRuntime::openGraphics(int w, int h, int d, int flags) {
	return 0;
}
void gxRuntime::closeGraphics(gxGraphics *graphics) {

}

gxFileSystem *gxRuntime::openFileSystem(int flags) {
	return 0;
}
void gxRuntime::closeFileSystem(gxFileSystem *filesys) {

}

gxTimer *gxRuntime::createTimer(int hertz) {
	return 0;
}
void gxRuntime::freeTimer(gxTimer *timer) {

}

gxRuntime *gxRuntime::openRuntime(const std::string &cmd_line, Debugger *debugger) {
	return 0;
}
void gxRuntime::closeRuntime(gxRuntime *runtime) {

}

void gxRuntime::asyncStop() {

}
void gxRuntime::asyncRun() {

}
void gxRuntime::asyncEnd() {

}

