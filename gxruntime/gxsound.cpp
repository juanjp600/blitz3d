
#include "std.h"
#include "gxsound.h"
#include "gxaudio.h"

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

static void streamOGG(const std::string &filename,bool isPanned,std::atomic<bool>& markForDeletion,ALuint source) {
	printf("Working!\n");
	ALenum format = 0; ALsizei freq = 0;
	int endian = 0;
	int bitStream;
	long bytes;
	char* arry = new char[4096];
	FILE *f;
	f=fopen(filename.c_str(),"rb");
	if (f==nullptr) {
		return;
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

	std::vector<char> bufData;
	bufData.clear();

	bool finalData = false;
	for (int i=0; i<4; i++) {
		ALuint buffer = 0; alGenBuffers(1,&buffer);
		bufData.clear();
		do {
			bytes = ov_read(&oggfile,tmparry,4096,endian,2,1,&bitStream);
			for (unsigned int i=0;i<bytes/(div*2);i++) {
				arry[i*2]=tmparry[i*div*2];
				arry[(i*2)+1]=tmparry[(i*div*2)+1];
				if (div>1) {
					arry[i*2]=tmparry[(i*div*2)+2];
					arry[(i*2)+1]=tmparry[(i*div*2)+3];
				}
			}
			if (bytes<=0) {
				finalData = true;
			} else {
				bufData.insert(bufData.end(),arry,arry+(bytes/div));
			}
		} while (bytes>0 && bufData.size()<4096*16);

		alBufferData(buffer,format,&bufData[0],bufData.size(),freq);
		alSourceQueueBuffers(source,1,&buffer);
	}
	alSourceRewind(source);
	alSourcePlay(source);

	finalData = false;

	//printf("a\n");
	while (!markForDeletion) {
		//printf("b\n");
		ALint buffersFree = 0;
		alGetSourcei(source,AL_BUFFERS_PROCESSED,&buffersFree);
		//printf("c ");
		printf(to_string(buffersFree).c_str());
		printf("\n");
		//printf("\n");
		while (buffersFree>0) {
			printf("Queueing buffer.\n");
			ALuint buffer = 0;
			alSourceUnqueueBuffers(source,1,&buffer);
			bufData.clear();
			do {
				bytes = ov_read(&oggfile,tmparry,4096,endian,2,1,&bitStream);
				for (unsigned int i=0;i<bytes/(div*2);i++) {
					arry[i*2]=tmparry[i*div*2];
					arry[(i*2)+1]=tmparry[(i*div*2)+1];
					if (div>1) {
						arry[i*2]=tmparry[(i*div*2)+2];
						arry[(i*2)+1]=tmparry[(i*div*2)+3];
					}
				}
				if (bytes<=0) {
					finalData = true;
				} else {
					bufData.insert(bufData.end(),arry,arry+(bytes/div));
				}
			} while (bytes>0 && bufData.size()<4096*16);

			alBufferData(buffer,format,&bufData[0],bufData.size(),freq);
			alSourceQueueBuffers(source,1,&buffer);
			if (finalData) {
				printf("Final data?\n");
				ALint loops = 0;
				alGetSourcei(source,AL_LOOPING,&loops);
				ALint isPlaying = 0;
				alGetSourcei(source,AL_PLAYING,&isPlaying);
				if (loops && isPlaying) {
					//source is set to loop:
					//set oggvorbis seek pos to 0
					//to restream the data
					finalData = false;
					ov_raw_seek(&oggfile,0);
					printf("Nope.\n");
				}
			}
			if (finalData) break; //we can't fill any more buffers
			alGetSourcei(source,AL_BUFFERS_PROCESSED,&buffersFree);
		}
		//printf("d\n");
		if (finalData) {
			ALint isPlaying = 0;
			alGetSourcei(source,AL_PLAYING,&isPlaying);
			while (isPlaying) {
				alGetSourcei(source,AL_PLAYING,&isPlaying);
			}
			break;
		}
		//printf("e\n");
	}

	ALint buffersFree = 0;
	alGetSourcei(source,AL_BUFFERS_PROCESSED,&buffersFree);
	ALuint* bufs;
	alSourceUnqueueBuffers(source,buffersFree,bufs);
	alDeleteBuffers(buffersFree,bufs);

	delete[] tmparry;
	delete[] arry;

	ov_clear(&oggfile);
}

gxSoundStream::gxSoundStream( gxAudio *a,const std::string& name ){
	audio=a; filename=name;
	markedForDeletion = false;
	setLoop( false );
	setVolume( 1.f );
	setPitch( 1.f );
	setRange( 100.f, 200.f );
	for (int i=0; i<8; i++) {
		streamThread[i]=0;
	}
}

gxSoundStream::~gxSoundStream(){
	markedForDeletion = true;
	for (int i=0; i<8; i++) {
		if (streamThread[i]){
			streamThread[i]->join();
			delete streamThread[i];
		}
	}
	audio->clearRelatedChannels(this);
}

gxChannel* gxSoundStream::play() {
	gxChannel* retVal = 0;
	for (int i=0; i<8; i++) {
		if (!streamThread[i]) {
			retVal = audio->reserveChannel(this,false);
			streamThread[i] = new std::thread(streamOGG,filename,false,std::ref(markedForDeletion),retVal->getALSource());
			break;
			//streamThread[i]->join();
			//streamThread[i]->detach();
		}
	}
	return retVal;
}

gxChannel* gxSoundStream::play3d(const float pos[3], const float vel[3]) {
	return play();
}

gxSoundStream* gxSoundStream::load(gxAudio* a, const std::string& name, bool use_3d) {
	gxSoundStream *sound=d_new gxSoundStream( a,name );
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
