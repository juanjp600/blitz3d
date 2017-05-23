
#include "std.h"
#include "gxaudio.h"

#include <vorbis/vorbisfile.h>
#include <ogg/ogg.h>

struct StaticChannel : public gxChannel{
	virtual void play()=0;
};

struct SoundChannel : public gxChannel{
	SoundChannel():source(-1){
	}
	void set( ALuint insource,gxSound* insound ){
		source=insource;
		sound=insound;
	}
	void stop(){
        isPaused = false;
        alSourceStop( source );
        alSourceRewind( source );
	}
	void setPaused( bool paused ){
        if (isPaused != paused) {
            if (paused) {
                alSourcePause( source );
            } else {
                alSourcePlay( source );
            }
        }
		isPaused = paused;
	}
	void setPitch( float pitch ){
        alSourcef( source,AL_PITCH,pitch );
	}
	void setVolume( float volume ){
        alSourcef( source,AL_GAIN,volume );
	}
	void setRange(float inNear, float inFar) {
		alSourcef( source,AL_REFERENCE_DISTANCE,inNear );
		alSourcef( source,AL_MAX_DISTANCE,inFar );
	}
	void set3d( const float pos[3],const float vel[3] ){
        alSource3f( source,AL_POSITION,pos[0],pos[1],pos[2] );
        alSource3f( source,AL_VELOCITY,vel[0],vel[1],vel[2] );
	}
	bool isPlaying(){
        ALint state;
        alGetSourcei( source, AL_SOURCE_STATE, &state);
		return state==AL_PLAYING || state==AL_PAUSED;
	}
	bool isRelated(gxSound* snd) {
		return snd==sound;
	}
private:
	ALuint source;
	gxSound* sound;
};

static set<gxSound*> sound_set;
static vector<gxChannel*> channels;

/*static int next_chan;
static vector<SoundChannel*> soundChannels;

static gxChannel *allocSoundChannel( int n ){

	SoundChannel *chan=0;
	for( int k=0;k<(int)soundChannels.size();++k ){
		chan=soundChannels[next_chan];
		if( !chan ){
			chan=soundChannels[next_chan]=d_new SoundChannel();
			channels.push_back(chan);
		}else if( chan->isPlaying() ){
			chan=0;
		}
		if( ++next_chan==soundChannels.size() ) next_chan=0;
		if( chan ) break;
	}

	if( !chan ){
		next_chan=soundChannels.size();
		soundChannels.resize(soundChannels.size()*2);
		for( int k=next_chan;k<(int)soundChannels.size();++k ) soundChannels[k]=0;
		chan=soundChannels[next_chan++]=d_new SoundChannel();
		channels.push_back( chan );
	}

	chan->set(n);
	return chan;
}*/

gxAudioAsyncLoadData::~gxAudioAsyncLoadData(){
	if (oggfile) { ov_clear(oggfile); delete oggfile; }
}

gxAudio::gxAudio( gxRuntime *r ):
runtime(r){
	//next_chan=0;
	//soundChannels.resize( SOURCE_COUNT );
	for( int k=0;k<SOURCE_COUNT;++k ) channels[k]=0;

    device = alcOpenDevice(0);
    context = alcCreateContext(device,0);
    alcMakeContextCurrent(context);

    for (int i=0;i<SOURCE_COUNT;i++){
        alGenSources(1,&sources[i]);
        alSourceStop(sources[i]);

        alSourcef(sources[i],AL_REFERENCE_DISTANCE,100.f);
        alSourcef(sources[i],AL_MAX_DISTANCE,200.f);
        alSourcef(sources[i],AL_GAIN,1.f);
    }

    listenerPos[0] = 0.f; listenerPos[1] = 0.f; listenerPos[2] = 0.f;
    listenerTarget[0] = 0.f; listenerTarget[1] = 0.f; listenerTarget[2] = 1.f;
    listenerUp[0] = 0.f; listenerUp[1] = 1.f; listenerUp[2] = 0.f;
    listenerVel[0] = 0.f; listenerVel[1] = 0.f; listenerVel[2] = 0.f;
    set3dListener(listenerPos,listenerVel,listenerTarget,listenerUp);
    alDistanceModel(AL_LINEAR_DISTANCE_CLAMPED);
}

gxAudio::~gxAudio(){
	//free all channels
    for (unsigned int i=0;i<SOURCE_COUNT;i++) {
        alDeleteSources(1,&sources[i]);
    }
	//for( ;channels.size();channels.pop_back() ) delete channels.back();
	//free all sound_set
	while( sound_set.size() ) freeSound( *sound_set.begin() );
	//soundChannels.clear();
    
    alcMakeContextCurrent(0);
	alcDestroyContext(context);
    alcCloseDevice(device);
}

gxChannel *gxAudio::play( gxSound* sound,bool loop ){
	ALuint sample = sound->getSample();
    gxChannel* channel = 0;
    int sourceInd = -1;
    for (int i=0;i<SOURCE_COUNT;i++){
        if (channels[i] == 0) {
            channels[i] = d_new SoundChannel();
            channel = channels[i];
            sourceInd = i;
        } else if (!channels[i]->isPlaying()) {
            delete channels[i];
            channels[i] = d_new SoundChannel();
            channel = channels[i];
            sourceInd = i;
        }
    }
    if (channel == 0) return 0;
    ((SoundChannel*)channel)->set(sources[sourceInd],sound);
    alSourcei(sources[sourceInd], AL_LOOPING,loop);
    alSourcei(sources[sourceInd], AL_BUFFER, sample);
    alSourcei(sources[sourceInd], AL_SOURCE_RELATIVE, AL_TRUE);
	alSource3f(sources[sourceInd], AL_POSITION, listenerPos[0],listenerPos[1],listenerPos[2]);
	alSource3f(sources[sourceInd], AL_VELOCITY, 0.f,0.f,0.f);
    alSourceRewind(sources[sourceInd]);
	alSourcePlay(sources[sourceInd]);
    return channel;
}

gxChannel *gxAudio::play3d( gxSound* sound,bool loop,const float pos[3],const float vel[3] ){
	ALuint sample = sound->getSample();
	gxChannel* channel = 0;
    int sourceInd = -1;
    for (int i=0;i<SOURCE_COUNT;i++){
        if (channels[i] == 0) {
            channels[i] = d_new SoundChannel();
            channel = channels[i];
            sourceInd = i;
        } else if (!channels[i]->isPlaying()) {
            delete channels[i];
            channels[i] = d_new SoundChannel();
            channel = channels[i];
            sourceInd = i;
        }
    }
    if (channel == 0) return 0;
    ((SoundChannel*)channel)->set(sources[sourceInd],sound);
    alSourcei(sources[sourceInd], AL_LOOPING,loop);
    alSourcei(sources[sourceInd], AL_BUFFER, sample);
    alSourcei(sources[sourceInd], AL_SOURCE_RELATIVE, AL_TRUE);
    alSource3f(sources[sourceInd], AL_POSITION, pos[0],pos[1],pos[2]);
    alSource3f(sources[sourceInd], AL_VELOCITY, vel[0],vel[1],vel[2]);
    alSourceRewind(sources[sourceInd]);
    alSourcePlay(sources[sourceInd]);
    return channel;
}

void gxAudio::clearRelatedChannels(gxSound* sound) {
	for (int i=0; i<32; i++) {
		if (channels[i]) {
			if (((SoundChannel*)channels[i])->isRelated(sound)){
				channels[i]->stop();
				delete channels[i]; channels[i]=0;
			}
		}
	}
}

bool gxAudio::verifyChannel(gxChannel* chan) {
	for (int i=0; i<32; i++) {
		if (channels[i]==chan) return true;
	}
	return false;
}

/*void gxAudio::pause(){
}

void gxAudio::resume(){
}*/

gxSound *gxAudio::loadSound( const string &f,bool use3d ){
    std::vector<char> bufData; ALenum format; ALsizei freq;
	if (tolower(f.substr(f.size()-4))!=".ogg") return 0;
    if (loadOGG(f,bufData,format,freq,use3d)) {
        ALuint sample = 0;
        alGenBuffers(1,&sample);
        alBufferData(sample,format,&bufData[0],static_cast<ALsizei>(bufData.size()),freq);
	    if( !sample ) return 0;

	    gxSound *sound=d_new gxSound( this,sample );
	    sound_set.insert( sound );
	    return sound;
    }
    return 0;
}

gxSound *gxAudio::loadSoundAsync( const string &f,bool use3d ){
	if (tolower(f.substr(f.size()-4))!=".ogg") return 0;
	gxSound *sound=d_new gxSound( this,loadOGG_asyncInit(f,use3d) );
	sound_set.insert( sound );
	return sound;
}

gxAudioAsyncLoadData* gxAudio::loadOGG_asyncInit(const std::string &filename,bool isPanned) {
	gxAudioAsyncLoadData* ald = new gxAudioAsyncLoadData();
	ald->buffer.resize(0);
	ald->endian = 0;
	//char* arry = new char[4096];
	FILE *f;
	f=fopen(filename.c_str(),"rb");
	if (f==nullptr) {
		return false;
	}
	vorbis_info *pInfo;
	ald->oggfile = new OggVorbis_File();
	ov_open(f,ald->oggfile,"",0);
	pInfo = ov_info(ald->oggfile,-1);
	if (pInfo->channels == 1) {
		ald->format = AL_FORMAT_MONO16;
	} else {
		ald->format = AL_FORMAT_STEREO16;
	}
	ald->freq = pInfo->rate;
	ald->div = 1;
	if (isPanned && ald->format==AL_FORMAT_STEREO16) {
		//OpenAL does not perform automatic panning or attenuation with stereo tracks
		ald->format = AL_FORMAT_MONO16;
		ald->div=2;
	}

	return ald;
}

bool gxAudio::loadOGG_asyncUpdate(gxAudioAsyncLoadData* ald) {
	bool retVal = false;

	char* arry = new char[4096];
	char* tmparry = new char[4096];

	int bitStream;
	int div = ald->div;
	long bytes = ov_read(ald->oggfile,tmparry,4096,ald->endian,2,1,&bitStream);
	if (bytes>0) {
		for (unsigned int i=0;i<bytes/(div*2);i++) {
			arry[i*2]=tmparry[i*div*2];
			arry[(i*2)+1]=tmparry[(i*div*2)+1];
			if (div>1) {
				arry[i*2]=tmparry[(i*div*2)+2];
				arry[(i*2)+1]=tmparry[(i*div*2)+3];
			}
		}
		ald->buffer.insert(ald->buffer.end(),arry,arry+(bytes/div));
	} else {
		ov_clear(ald->oggfile);
		delete ald->oggfile;
		ald->oggfile = nullptr;
		retVal = true;
	}

	delete[] tmparry;
	delete[] arry;

	return retVal;
}

bool gxAudio::loadOGG(const std::string &filename,std::vector<char> &buffer,ALenum &format,ALsizei &freq,bool isPanned) {
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

gxSound *gxAudio::verifySound( gxSound *s ){
	return sound_set.count( s )  ? s : 0;
}

void gxAudio::freeSound( gxSound *s ){
	if( sound_set.erase( s ) ) delete s;
}

/*void gxAudio::setPaused( bool paused ){
	FSOUND_SetPaused( FSOUND_ALL,paused );
}

void gxAudio::setVolume( float volume ){
}*/

void gxAudio::set3dOptions( float roll,float dopp,float dist ){
    for (int i=0;i<SOURCE_COUNT;i++){
        alSourcef(sources[i],AL_ROLLOFF_FACTOR,roll);
        alSourcef(sources[i],AL_DOPPLER_FACTOR,dopp);
        alSourcef(sources[i],AL_MAX_DISTANCE,dist);
    }
}

void gxAudio::set3dListener( const float pos[3],const float vel[3],const float forward[3],const float up[3] ){
	alListener3f(AL_POSITION,pos[0],pos[1],pos[2]);
    float orientation[] = {forward[0],forward[1],forward[2],up[0],up[1],up[2]};
    alListenerfv(AL_ORIENTATION,orientation);
    listenerPos[0] = pos[0]; listenerPos[1] = pos[1]; listenerPos[2] = pos[2];
    listenerTarget[0] = forward[0]; listenerTarget[1] = forward[1]; listenerTarget[2] = forward[2];
    listenerUp[0] = up[0]; listenerUp[1] = up[1]; listenerUp[2] = up[2];
    //TODO: add velocity, if at all possible
}

const float* gxAudio::get3dListenerPos(){
    return listenerPos;
}

const float* gxAudio::get3dListenerTarget(){
    return listenerTarget;
}

const float* gxAudio::get3dListenerUp(){
    return listenerUp;
}
