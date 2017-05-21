
#ifndef GXCHANNEL_H
#define GXCHANNEL_H

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
	virtual void setPaused( bool paused )=0;
	virtual void setPitch( float pitch )=0;
	virtual void setVolume( float volume )=0;
	virtual void set3d( const float pos[3],const float vel[3] )=0;

	virtual bool isPlaying()=0;
};

#endif