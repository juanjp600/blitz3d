
#include "std.h"
#include "gxchannel.h"

#include "gxaudio.h"
#include "gxsound.h"

#include <ogg/ogg.h>
#include <vorbis/vorbisfile.h>

gxChannel::~gxChannel(){
}

SampleChannel::SampleChannel(gxSoundSample* insound){
	sound = insound;
	source = 0;
}

void SampleChannel::setSource( ALuint insource ){
	source=insource;

	alSourcei(source, AL_SOURCE_RELATIVE, AL_TRUE);

	alSourceRewind(source);
	alSourceStop(source);
}

void SampleChannel::stop(){
	isPaused = false;
	alSourceStop( source );
	alSourceRewind( source );
}

void SampleChannel::setLoop(bool loop) {
	alSourcei(source,AL_LOOPING,loop);
}

void SampleChannel::setPaused( bool paused ){
	if (isPaused != paused) {
		if (paused) {
			alSourcePause( source );
		} else {
			alSourcePlay( source );
		}
	}
	isPaused = paused;
}

void SampleChannel::setPitch( float pitch ){
	alSourcef( source,AL_PITCH,pitch );
}

void SampleChannel::setVolume( float volume ){
	alSourcef( source,AL_GAIN,volume );
}

void SampleChannel::setRange(float inNear, float inFar) {
	alSourcef( source,AL_REFERENCE_DISTANCE,inNear*gxAudio::posScale );
	alSourcef( source,AL_MAX_DISTANCE,inFar*gxAudio::posScale );
}

void SampleChannel::set3d( const float pos[3],const float vel[3] ){
	alSourcei(source, AL_SOURCE_RELATIVE, AL_FALSE);

	alSource3f( source,AL_POSITION,pos[0]*gxAudio::posScale,pos[1]*gxAudio::posScale,pos[2]*gxAudio::posScale );
	alSource3f( source,AL_VELOCITY,vel[0],vel[1],vel[2] );
}

bool SampleChannel::isPlaying(){
	ALint state = 0;
	alGetSourcei( source, AL_SOURCE_STATE, &state);
	return state==AL_PLAYING || state==AL_PAUSED;
}

bool SampleChannel::isRelated(gxSound* snd) {
	return snd==sound;
}

bool SampleChannel::canDispose() {
	return !isPlaying();
}

ALuint SampleChannel::getALSource() {
	return source;
}

void SampleChannel::setTime( float seconds ){
    ALint freq;
    alGetBufferi(sound->getSample(),AL_FREQUENCY,&freq);
    alSourcei(source,AL_SAMPLE_OFFSET,seconds*freq);
}

StreamChannel::StreamChannel(gxSoundStream* insound){
	sound = insound;
	source = 0;

	thread = 0;

	playing = false;
	markedForDeletion = false;
	seek = -1.f;
}
void StreamChannel::setSource( ALuint insource ){
	source=insource;

	alSourceStop(source);
	alSourceRewind(source);

	alSourcei(source,AL_LOOPING,false);
}
void StreamChannel::setLoop(bool loop) {
	looping = loop;
}
void StreamChannel::stop(){
	alSourceStop(source);
	alSourceRewind(source);
	playing = false;
}
void StreamChannel::setPaused( bool paused ){
	if (isPaused != paused) {
		if (paused) {
			alSourcePause( source );
		} else {
			alSourcePlay( source );
		}
	}
	isPaused = paused;
}
void StreamChannel::setPitch( float pitch ){
	alSourcef( source,AL_PITCH,pitch );
}
void StreamChannel::setVolume( float volume ){
	alSourcef( source,AL_GAIN,volume );
}
void StreamChannel::setRange(float inNear, float inFar) {
	alSourcef( source,AL_REFERENCE_DISTANCE,inNear*gxAudio::posScale );
	alSourcef( source,AL_MAX_DISTANCE,inFar*gxAudio::posScale );
}
void StreamChannel::set3d( const float pos[3],const float vel[3] ){
	alSourcei(source, AL_SOURCE_RELATIVE, AL_FALSE);

	alSource3f( source,AL_POSITION,pos[0]*gxAudio::posScale,pos[1]*gxAudio::posScale,pos[2]*gxAudio::posScale );
	alSource3f( source,AL_VELOCITY,vel[0],vel[1],vel[2] );
}
bool StreamChannel::isPlaying(){
	return playing && !markedForDeletion;
}
bool StreamChannel::isRelated(gxSound* snd) {
	return snd==sound;
}
bool StreamChannel::canDispose() {
	return markedForDeletion;
}
ALuint StreamChannel::getALSource() {
	return source;
}

static void streamOGG(const std::string &filename,bool isPanned,
                      std::atomic<bool>& isLooping,
                      std::atomic<bool>& isPlaying,
                      std::atomic<float>& seek,
                      std::atomic<bool>& markForDeletion,
                      gxChannel* channel) {
	ALuint source = channel->getALSource();
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

	ALuint buffers[4];

	bool finalData = false;
	for (int i=0; i<4; i++) {
		buffers[i] = 0;
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

		if (bufData.size()>0) {
			ALuint buffer = 0; alGenBuffers(1,&buffer);
			buffers[i] = buffer;

			alBufferData(buffer,format,&bufData[0],bufData.size(),freq);
			alSourceQueueBuffers(source,1,&buffer);
		}
	}
	alSourceRewind(source);
	alSourcePlay(source);

	finalData = false;

	while (true) {
		if (!isPlaying) {
			break;
		}
		std::this_thread::sleep_for(80ms);
		ALint buffersFree = 0;
		alGetSourcei(source,AL_BUFFERS_PROCESSED,&buffersFree);

		if (seek>=0.f) {
			ov_time_seek(&oggfile,seek);

			alSourceStop(source); //makes all buffers ready to dequeue

			ALuint tbuffers[4];

			ALint buffersFree = 0;
			alGetSourcei(source,AL_BUFFERS_PROCESSED,&buffersFree);

			//dequeue all buffers
			for (int i=0;i<buffersFree;i++) {
				alSourceUnqueueBuffers(source,1,&(tbuffers[i]));
			}
			//place them back in with the new data
			for (int i=0;i<buffersFree;i++) {
				bufData.clear();
				do {
					bytes = ov_read(&oggfile,tmparry,4096,endian,2,1,&bitStream);
					if (bytes<=0 && isLooping) {
						ov_raw_seek(&oggfile,0);
						bytes = ov_read(&oggfile,tmparry,4096,endian,2,1,&bitStream);
					}
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

				if (bufData.size()>0) {
					ALuint buffer = tbuffers[i];
					alBufferData(buffer,format,&bufData[0],bufData.size(),freq);
					alSourceQueueBuffers(source,1,&buffer);
				}
			}

			alSourceRewind(source);
			alSourcePlay(source); //replay the source

			seek = -1.f;
		} else {
			while (buffersFree>0) {
				bufData.clear();
				do {
					bytes = ov_read(&oggfile,tmparry,4096,endian,2,1,&bitStream);
					if (bytes<=0 && isLooping) {
						ov_raw_seek(&oggfile,0);
						bytes = ov_read(&oggfile,tmparry,4096,endian,2,1,&bitStream);
					}
					if (bytes>0) {
						for (unsigned int i=0;i<bytes/(div*2);i++) {
							arry[i*2]=tmparry[i*div*2];
							arry[(i*2)+1]=tmparry[(i*div*2)+1];
							if (div>1) {
								arry[i*2]=tmparry[(i*div*2)+2];
								arry[(i*2)+1]=tmparry[(i*div*2)+3];
							}
						}
						bufData.insert(bufData.end(),arry,arry+(bytes/div));
					} else {
						finalData = true;
					}
				} while (bytes>0 && bufData.size()<4096*16);

				if (bufData.size()>0) {
					ALuint buffer = 0;
					alSourceUnqueueBuffers(source,1,&buffer);
					
					alBufferData(buffer,format,&bufData[0],bufData.size(),freq);
					alSourceQueueBuffers(source,1,&buffer);
				}
				if (seek >= 0.f) {
					alSourceStop(source);
					break;
				}
				
				if (finalData) break; //we can't fill any more buffers

				alGetSourcei(source,AL_BUFFERS_PROCESSED,&buffersFree);
				std::this_thread::sleep_for(10ms);
			}
		}
		if (finalData) {
			ALint playing = 0;
			alGetSourcei(source,AL_SOURCE_STATE,&playing);
			while (playing==AL_PLAYING || playing==AL_PAUSED) {
				std::this_thread::sleep_for(80ms);
				alGetSourcei(source,AL_SOURCE_STATE,&playing);
				if (seek>=0.f) {
					break;
				}
			}
			if (seek>=0.f) {
				finalData = false;
			}
			if (finalData) {
				isPlaying = false;
				break;
			}
		}
	}

	alSourceStop(source);
	alSourceRewind(source);

	ALint buffersFree; ALuint *tbuf = 0;
	alGetSourcei(source,AL_BUFFERS_PROCESSED,&buffersFree);
	alSourceUnqueueBuffers(source,buffersFree,tbuf);

	alSourcei(source,AL_BUFFER,0);

	for (int i=0; i<4; i++) {
		if (buffers[i]) { alDeleteBuffers(1,&(buffers[i])); }
	}

	delete[] tmparry;
	delete[] arry;

	ov_clear(&oggfile);

	markForDeletion = true;
}

void StreamChannel::createThread(const std::string &filename,bool is_3d) {
	playing = true;
	markedForDeletion = false;
    seek = -1.f;
	thread = new std::thread(streamOGG,filename,is_3d,
                                       std::ref(looping),
                                       std::ref(playing),
                                       std::ref(seek),
                                       std::ref(markedForDeletion),
                                       this);
}

void StreamChannel::setTime( float seconds ){
    seek = seconds; alSourceStop(source);
}

StreamChannel::~StreamChannel() {
	markedForDeletion = true; playing = false;
	if (thread) {
		thread->join();
		delete thread;
		thread = 0;
	}
}

