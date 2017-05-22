
#include "std.h"
#include "gxsound.h"
#include "gxaudio.h"

gxSound::gxSound( gxAudio *a,ALuint s ){
    audio=a; sample=s;
	setLoop( false );
	setVolume( 1.f );
	setPitch( 1.f );
	asyncLoadData = 0;
	ready = true;
}

gxSound::gxSound( gxAudio *a,gxAudioAsyncLoadData *ald ){
	audio=a;
	asyncLoadData=ald;
	setLoop( false );
	setVolume( 1.f );
	setPitch( 1.f );
	ready = false;
}

gxSound::~gxSound(){
	if (ready) {
		alDeleteBuffers(1,&sample);
	} else {
		delete asyncLoadData;
	}
}

gxChannel *gxSound::play(){
	if (!ready) return 0;
    gxChannel* retVal = audio->play( sample,def_loop );
    retVal->setPitch(def_pitch);
    retVal->setVolume(def_gain);
	return retVal;
}

gxChannel *gxSound::play3d( const float pos[3],const float vel[3] ){
	if (!ready) return 0;
	gxChannel* retVal = audio->play3d( sample,def_loop,pos,vel );
    retVal->setPitch(def_pitch);
    retVal->setVolume(def_gain);
	return retVal;
}

void gxSound::prepare() {
	if (ready) return;
	if (audio->loadOGG_asyncUpdate(asyncLoadData)) {
		sample = 0;
		alGenBuffers(1,&sample);
		alBufferData(sample,asyncLoadData->format,&asyncLoadData->buffer[0],static_cast<ALsizei>(asyncLoadData->buffer.size()),asyncLoadData->freq);
		delete asyncLoadData;
		ready = true;
	}
}

bool gxSound::isReady() {
	return ready;
}

void gxSound::setLoop( bool loop ){
	def_loop = loop;
}

void gxSound::setPitch( float pitch ){
	def_pitch = pitch;
}

void gxSound::setVolume( float volume ){
	def_gain=volume;
}

