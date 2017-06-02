
#include "std.h"
#include "gxsound.h"
#include "gxaudio.h"
#include "gxchannel.h"

#include <vorbis/vorbisfile.h>
#include <ogg/ogg.h>

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

gxSoundSample* gxSoundSample::load( gxAudio *a,const std::string &filename,bool use_3d ) {
	std::vector<char> bufData; ALenum format; ALsizei freq;
	if (tolower(filename.substr(filename.size()-4))!=".ogg") return 0;
	if (loadOGG(filename,bufData,format,freq,use_3d)) {
		ALuint sample = 0;
		alGenBuffers(1,&sample);
		alBufferData(sample,format,&bufData[0],static_cast<ALsizei>(bufData.size()),freq);
		if( !sample ) return 0;

		gxSoundSample *sound=d_new gxSoundSample( a,sample );
		a->sound_set.insert( sound );
		return sound;
	}
	return 0;
}

bool gxSoundSample::loadOGG(const std::string &filename,std::vector<char> &buffer,ALenum &format,ALsizei &freq,bool isPanned) {
	buffer.resize(0);
	int endian = 0;
	int bitStream;
	long bytes;
	char* arry = new char[4096];
	FILE *f;
	f=fopen(filename.c_str(),"rb");
	if (f==nullptr) {
		return false;
	}
	vorbis_info *pInfo;
	OggVorbis_File oggfile;
	ov_open(f,&oggfile,"",0);
	pInfo = ov_info(&oggfile,-1);
	if (pInfo->channels == 1) {
		format = AL_FORMAT_MONO16;
	} else {
		format = AL_FORMAT_STEREO16;
	}
	freq = pInfo->rate;
	int div = 1;
	if (isPanned && format==AL_FORMAT_STEREO16) {
		//OpenAL does not perform automatic panning or attenuation with stereo tracks
		format = AL_FORMAT_MONO16;
		div=2;
	}
	char* tmparry = new char[4096];
	do {
		bytes = ov_read(&oggfile,tmparry,4096,endian,2,1,&bitStream);
		for (unsigned int i=0;i<bytes/(div*2);i++) {
			arry[i*2]=tmparry[i*div*2];
			arry[(i*2)+1]=tmparry[(i*div*2)+1];
			if (div>1) {
				arry[i*2]=tmparry[(i*div*2)+2];
				arry[(i*2)+1]=tmparry[(i*div*2)+3];
			}
			//TODO: find out how to properly convert stereo to mono
		}
		buffer.insert(buffer.end(),arry,arry+(bytes/div));
	} while (bytes>0);

	delete[] tmparry;
	delete[] arry;

	ov_clear(&oggfile);

	return true;
}

void gxSoundSample::free() {
	audio->sound_set.erase( this );
	delete this;
}

ALuint gxSoundSample::getSample() { return sample; }

gxChannel *gxSoundSample::play(){
    SampleChannel* retVal = new SampleChannel(this);
	if (!audio->reserveChannel( retVal ) ) {
		delete retVal;
		return 0;
	}
	retVal->setLoop(def_loop);
	float pos[3]={0.f,0.f,0.f}; float vel[3]={0.f,0.f,0.f};
	retVal->set3d(pos,vel);
	alSourcei(retVal->getALSource(),AL_SOURCE_RELATIVE,AL_TRUE);
    retVal->setPitch(def_pitch);
    retVal->setVolume(def_gain);
	retVal->setRange(def_range_near,def_range_far);
	alSourcei(retVal->getALSource(), AL_BUFFER, sample);
	alSourcePlay(retVal->getALSource());
	return retVal;
}

/*gxChannel *gxSoundSample::play3d( const float pos[3],const float vel[3] ){
	SampleChannel* retVal = new SampleChannel(this);
	if (!audio->reserveChannel( retVal ) ) {
		delete retVal;
		return 0;
	}
	retVal->setLoop(def_loop);
	retVal->set3d(pos,vel);
    retVal->setPitch(def_pitch);
    retVal->setVolume(def_gain);
	retVal->setRange(def_range_near,def_range_far);
	alSourcei(retVal->getALSource(), AL_BUFFER, sample);
	alSourcePlay(retVal->getALSource());
	return retVal;
}*/

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

gxSoundStream::gxSoundStream( gxAudio *a,bool use_3d,const std::string& name ){
	audio=a; filename=name; is_3d=use_3d;
	setLoop( false );
	setVolume( 1.f );
	setPitch( 1.f );
	setRange( 100.f, 200.f );
}

gxSoundStream::~gxSoundStream(){
	audio->clearRelatedChannels(this);
}

gxChannel* gxSoundStream::play() {
	StreamChannel* retVal = new StreamChannel(this);
	if (!audio->reserveChannel( retVal ) ) {
		delete retVal;
		return 0;
	}
	retVal->setLoop(def_loop);
	float pos[3]={0.f,0.f,0.f}; float vel[3]={0.f,0.f,0.f};
	retVal->set3d(pos,vel);
	alSourcei(retVal->getALSource(),AL_SOURCE_RELATIVE,AL_TRUE);
	retVal->setPitch(def_pitch);
	retVal->setVolume(def_gain);
	retVal->setRange(def_range_near,def_range_far);
	retVal->createThread(filename,is_3d);
	
	return retVal;
}

/*gxChannel* gxSoundStream::play3d(const float pos[3], const float vel[3]) {
	StreamChannel* retVal = new StreamChannel(this);
	if (!audio->reserveChannel( retVal ) ) {
		delete retVal;
		return 0;
	}
	retVal->setLoop(def_loop);
	retVal->set3d(pos,vel);
	retVal->setPitch(def_pitch);
	retVal->setVolume(def_gain);
	retVal->setRange(def_range_near,def_range_far);
	retVal->createThread(filename,is_3d);

	return retVal;
}*/

gxSoundStream* gxSoundStream::load(gxAudio* a, const std::string& name, bool use_3d) {
	gxSoundStream *sound=d_new gxSoundStream( a,use_3d,name );
	a->sound_set.insert( sound );
	return sound;
}

void gxSoundStream::free() {
	audio->sound_set.erase( this );
	delete this;
}

void gxSoundStream::setLoop( bool loop ){
	def_loop = loop;
}

void gxSoundStream::setPitch( float pitch ){
	def_pitch = pitch;
}

void gxSoundStream::setVolume( float volume ){
	def_gain=volume;
}

void gxSoundStream::setRange(float inNear, float inFar) {
	def_range_near=inNear;
	def_range_far=inFar;
}
