
#include "std.h"
#include "gxsound.h"
#include "gxaudio.h"

gxSoundSample::gxSoundSample( gxAudio *a,ALuint s ){
    audio=a; sample=s;
	setLoop( false );
	setVolume( 1.f );
	setPitch( 1.f );
	setRange( 100.f, 200.f );
}

gxSoundSample::~gxSoundSample(){
	alDeleteBuffers(1,&sample);
	audio->clearRelatedChannels(this);
}

gxSoundSample* gxSoundSample::load( const std::string &filename,bool use_3d ) {

}

ALuint gxSoundSample::getSample() { return sample; }

gxChannel *gxSoundSample::play(){
    gxChannel* retVal = audio->reserveChannel( this,def_loop );
	if (!retVal) return 0;
    retVal->setPitch(def_pitch);
    retVal->setVolume(def_gain);
	retVal->setRange(def_range_near,def_range_far);
	alSourcei(retVal->getALSource(), AL_BUFFER, sample);
	alSourcePlay(retVal->getALSource());
	return retVal;
}

gxChannel *gxSoundSample::play3d( const float pos[3],const float vel[3] ){
	gxChannel* retVal = audio->reserveChannel( this,def_loop,pos,vel );
	if (!retVal) return 0;
    retVal->setPitch(def_pitch);
    retVal->setVolume(def_gain);
	retVal->setRange(def_range_near,def_range_far);
	alSourcei(retVal->getALSource(), AL_BUFFER, sample);
	alSourcePlay(retVal->getALSource());
	return retVal;
}

void gxSoundSample::setLoop( bool loop ){
	def_loop = loop;
}

void gxSoundSample::setPitch( float pitch ){
	def_pitch = pitch;
}

void gxSoundSample::setVolume( float volume ){
	def_gain=volume;
}

void gxSoundSample::setRange(float inNear, float inFar) {
	def_range_near=inNear;
	def_range_far=inFar;
}

