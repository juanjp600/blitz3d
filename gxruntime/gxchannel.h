
#ifndef GXCHANNEL_H
#define GXCHANNEL_H

#include <thread>
#include <atomic>

#include <al.h>
#include <alc.h>

class gxChannel{
protected:
    bool isPaused = false;
	/***** GX INTERFACE *****/
public:
	virtual ~gxChannel();
	//modifiers
	virtual void stop()=0;
	virtual void setLoop( bool loop )=0;
	virtual void setPaused( bool paused )=0;
	virtual void setPitch( float pitch )=0;
	virtual void setVolume( float volume )=0;
	virtual void setRange( float inNear,float inFar )=0;
	virtual void set3d( const float pos[3],const float vel[3] )=0;
	virtual void setSource( ALuint insource )=0;
    virtual void setTime( float seconds )=0;

	virtual bool isPlaying()=0;
	virtual bool isRelated(class gxSound* snd)=0;
	virtual bool canDispose()=0;

	virtual ALuint getALSource()=0;
};

class SampleChannel : public gxChannel{
public:
	SampleChannel(class gxSoundSample* insound);
	void setSource( ALuint insource );
	void stop();
	void setLoop(bool loop);
	void setPaused( bool paused );
	void setPitch( float pitch );
	void setVolume( float volume );
	void setRange(float inNear, float inFar);
	void set3d( const float pos[3],const float vel[3] );
    void setTime( float seconds );
	bool isPlaying();
	bool isRelated(gxSound* snd);
	bool canDispose();
	ALuint getALSource();
private:
	//~SampleChannel();
	ALuint source;
	class gxSoundSample* sound;
};

class StreamChannel : public gxChannel{
public:
	StreamChannel(class gxSoundStream* insound);
	void setSource( ALuint insource );
	void setLoop(bool loop);
	void stop();
	void setPaused( bool paused );
	void setPitch( float pitch );
	void setVolume( float volume );
	void setRange(float inNear, float inFar);
	void set3d( const float pos[3],const float vel[3] );
	bool isPlaying();
	bool isRelated(gxSound* snd);
	bool canDispose();
    void setTime( float seconds );
	ALuint getALSource();
	void createThread(const std::string &filename,bool is_3d);
	void kill();
	~StreamChannel();
private:
	ALuint source;
	class gxSoundStream* sound;
	std::thread* thread;
    
    //TODO: consider not using a ton of atomics
	std::atomic<bool> markedForDeletion;
	std::atomic<bool> playing;
	std::atomic<bool> looping;
    std::atomic<float> seek;
};


#endif