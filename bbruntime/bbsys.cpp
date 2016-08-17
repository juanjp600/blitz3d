
#include "std.h"
#include "bbsys.h"

bool debug;
gxRuntime *gx_runtime;

std::vector<std::string> errorLog;
BBStr* bbErrorLog( ){
	if (errorLog.size()==0) { return d_new BBStr(""); }
	BBStr* retVal=d_new BBStr(errorLog[0].c_str());
	errorLog.erase(errorLog.begin());
	return retVal;
}
