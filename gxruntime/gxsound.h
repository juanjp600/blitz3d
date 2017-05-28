
#ifndef GXSOUND_H
#define GXSOUND_H

#include "gxchannel.h"

#include <thread>
#include <atomic>

class gxAudio;

class gxSound{
public:
	gxAudio *audio;

protected:
	float def_gain,def_pitch;
    bool def_loop;
	float def_range_near;
	float def_range_far;
	float pos[3],vel[3];

	/***** GX INTERFACE *****/
public:
	//actions
	virtual gxChannel *play() =0;
	//virtual gxChannel *play3d( const float pos[3],const float vel[3] ) =0;

	//modifiers
	virtual void setLoop( bool loop ) =0;
	virtual void setPitch( float pitch ) =0;
	virtual void setVolume( float volume ) =0;
	virtual void setRange( float inNear,float inFar ) =0;

	//allocation
	virtual void free() =0;
};

class gxSoundSample : public gxSound {
public:
	gxSoundSample( gxAudio *audio,ALuint sample );
	~gxSoundSample();

private:
	ALuint sample;

	static bool loadOGG(const std::string &filename,std::vector<char> &buffer,ALenum &format,ALsizei &freq,bool isPanned);

	/***** GX INTERFACE *****/
public:
	//actions
	gxChannel *play();
	//gxChannel *play3d( const float pos[3],const float vel[3] );

	ALuint getSample();

	//modifiers
	void setLoop( bool loop );
	void setPitch( float pitch );
	void setVolume( float volume );
	void setRange( float inNear,float inFar );

	//allocation
	static gxSoundSample* load(gxAudio *a,const std::string &filename,bool use_3d);
	void free();
};

class gxSoundStream : public gxSound {
public:
	gxSoundStream( gxAudio *audio,bool use_3d,const std::string &filename );
	~gxSoundStream();

private:
	bool is_3d;
	std::string filename;

public:
	//actions
	gxChannel *play();
	//gxChannel *play3d( const float pos[3],const float vel[3] );

	//modifiers
	void setLoop( bool loop );
	void setPitch( float pitch );
	void setVolume( float volume );
	void setRange( float inNear,float inFar );

	//allocation
	static gxSoundStream* load(gxAudio *a,const std::string &filename,bool use_3d);
	void free();
};

#endif
