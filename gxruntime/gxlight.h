
#ifndef GXLIGHT_H
#define GXLIGHT_H

#include "std.h"

class gxScene;

class gxLight{
public:
	gxLight( gxScene *scene,int type );
	~gxLight();

	float color[3];
private:
	gxScene *scene;

	/***** GX INTERFACE *****/
public:
	enum{
		LIGHT_DISTANT=1,LIGHT_POINT=2,LIGHT_SPOT=3
	};
	void setRange( float range );
	void setColor( const float rgb[3] ){ memcpy( color,rgb,12 ); }
	void setPosition( const float pos[3] );
	void setDirection( const float dir[3] );
	void setConeAngles( float inner,float outer );

	void getColor( float rgb[3] ){ memcpy( rgb,color,12 ); }
};

#endif
