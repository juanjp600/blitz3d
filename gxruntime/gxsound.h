
#ifndef GXSOUND_H
#define GXSOUND_H

#include "gxchannel.h"

class gxAudio;
struct gxAudioAsyncLoadData;

class gxSound{
public:
	gxAudio *audio;

	gxSound( gxAudio *audio,ALuint sample );
	gxSound( gxAudio *audio,gxAudioAsyncLoadData *asyncLoadData );
	~gxSound();

private:
	float def_gain,def_pitch;
    bool def_loop;
	ALuint sample;
	float pos[3],vel[3];

	//async loading data
	bool ready;
	gxAudioAsyncLoadData *asyncLoadData;

	/***** GX INTERFACE *****/
public:
	//actions
	gxChannel *play();
	gxChannel *play3d( const float pos[3],const float vel[3] );

	//async loading
	void prepare();
	bool isReady();

	//modifiers
	void setLoop( bool loop );
	void setPitch( float pitch );
	void setVolume( float volume );
};

#endif
