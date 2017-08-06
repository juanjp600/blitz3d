
#ifndef BBSYS_H
#define BBSYS_H

#include "basic.h"
#include "../gxruntime/gxruntime.h"

extern bool debug;
extern gxRuntime *gx_runtime;

extern std::vector<std::string> errorLog;
BBStr * bbErrorLog( );

struct bbEx{
	const char *err;
	bbEx( const char *e ):err(e){
		if( e ) {
            string panicStr = e;
            panicStr+="\n\nStack trace:\n";
            try {
                string tmp = "";
                for (int i=0; i<blockTraces.size(); i++) {
                    tmp = "- "+blockTraces[i].file+", line "+to_string(blockTraces[i].lineTrace)+"\n"+tmp;
                }
                panicStr+=tmp;
            } catch (exception e){
                panicStr+="ERROR RETRIEVING FULL STACKTRACE: ";
                panicStr+=e.what();
                panicStr+="\n";
            } catch (...) {
                panicStr+="ERROR RETRIEVING FULL STACKTRACE\n";
            }

            gx_runtime->debugError( panicStr.c_str() );
        }
	}
};

#define RTEX( _X_ ) throw bbEx( _X_ );

#endif