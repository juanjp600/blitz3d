
#ifndef GXAUDIO_H
#define GXAUDIO_H

#include <string>
#include <set>

#include "gxsound.h"

class gxRuntime;

class gxAudio{
public:
	gxRuntime *runtime;

	gxAudio( gxRuntime *runtime );
	~gxAudio();

	std::set<gxSound*> sound_set;

    //sample = buffer
	bool reserveChannel(gxChannel* channel);

	void clearRelatedChannels( gxSound* sound );
	bool verifyChannel( gxChannel* chan );

	static const float posScale;
private:
    ALCdevice* device;
    ALCcontext* context;

    static const int SOURCE_COUNT = 32;

    int bufferCount = 0;
    ALuint sources[SOURCE_COUNT];
    gxChannel* channels[SOURCE_COUNT];

    float listenerPos[3];
    float listenerTarget[3];
    float listenerUp[3];
    float listenerVel[3];
	/***** GX INTERFACE *****/
public:
	gxSound *verifySound( gxSound *sound );
	
	void set3dOptions( float roll,float dopp,float dist );

	void set3dListener( const float pos[3],const float vel[3],const float forward[3],const float up[3] );
    const float* get3dListenerPos();
    const float* get3dListenerTarget();
	const float* get3dListenerUp();
	const float* get3dListenerVel();
};

#endif