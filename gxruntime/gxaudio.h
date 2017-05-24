
#ifndef GXAUDIO_H
#define GXAUDIO_H

#include <string>

#include "gxsound.h"

class gxRuntime;

class gxAudio{
public:
	gxRuntime *runtime;

	gxAudio( gxRuntime *runtime );
	~gxAudio();

    //sample = buffer
	gxChannel *reserveChannel(gxSound* sound,bool loop,const float pos[3]=0,const float vel[3]=0);
	//gxChannel *play( gxSound* sound,ALuint sample,bool loop );
	//gxChannel *play3d( gxSound* sound,ALuint sample,bool loop,const float pos[3],const float vel[3] );

	void clearRelatedChannels( gxSound* sound );
	bool verifyChannel( gxChannel* chan );

	/*void pause();
	void resume();*/

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
	//gxSound *loadSound( const std::string &filename,bool use_3d );
    bool loadOGG(const std::string &filename,std::vector<char> &buffer,ALenum &format,ALsizei &freq,bool isPanned);
	
	gxSound *verifySound( gxSound *sound );
	//void freeSound( gxSound *sound );

	/*void setPaused( bool paused );	//master pause
	void setVolume( float volume );	//master volume*/

	void set3dOptions( float roll,float dopp,float dist );

	void set3dListener( const float pos[3],const float vel[3],const float forward[3],const float up[3] );
    const float* get3dListenerPos();
    const float* get3dListenerTarget();
    const float* get3dListenerUp();
};

#endif