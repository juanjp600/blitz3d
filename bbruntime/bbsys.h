
#ifndef BBSYS_H
#define BBSYS_H

#include "basic.h"
#include "../gxruntime/gxruntime.h"
#include "../gxruntime/gxdevice.h"

extern bool debug;
extern gxRuntime *gx_runtime;

extern std::vector<std::string> errorLog;
BBStr * bbErrorLog( );

struct bbEx{
	const char *err;
	bbEx( const char *e ):err(e){
		if( e ) gx_runtime->debugError( e );
	}
};

#define RTEX( _X_ ) throw bbEx( _X_ );

#endif