
#include "std.h"
#include "gxaudio.h"

const float gxAudio::posScale = 0.01f;

gxAudio::gxAudio( gxRuntime *r ):
runtime(r){
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
	//free all sound_set
	while( sound_set.size() ) (*sound_set.begin())->free();

	//free all channels
	for (unsigned int i=0;i<SOURCE_COUNT;i++) {
		if (channels[i]) {
			channels[i]->stop();
			while (!channels[i]->canDispose()) {}
			delete channels[i];
		}
		alDeleteSources(1,&sources[i]);
	}
    
    alcMakeContextCurrent(0);
	alcDestroyContext(context);
    alcCloseDevice(device);
}

bool gxAudio::reserveChannel(gxChannel* channel) {
	int sourceInd = -1;
	for (int i=0;i<SOURCE_COUNT;i++){
		if (channels[i] == 0) {
			channels[i] = channel;
			sourceInd = i;
			break;
		} else if (channels[i]->canDispose()) {
			delete channels[i];
			channels[i] = channel;
			sourceInd = i;
			break;
		}
	}
	if (sourceInd < 0) return false;
	channel->setSource(sources[sourceInd]);
	return true;
}

void gxAudio::clearRelatedChannels(gxSound* sound) {
	for (int i=0; i<32; i++) {
		if (channels[i]) {
			if (channels[i]->isRelated(sound)){
				channels[i]->stop();
			}
		}
	}
}

bool gxAudio::verifyChannel(gxChannel* chan) {
	if (!chan) return false;
	for (int i=0; i<32; i++) {
		if (channels[i]==chan) return true;
	}
	return false;
}

gxSound *gxAudio::verifySound( gxSound *s ){
	return sound_set.count( s )  ? s : 0;
}

void gxAudio::set3dOptions( float roll,float dopp,float dist ){
    for (int i=0;i<SOURCE_COUNT;i++){
        alSourcef(sources[i],AL_ROLLOFF_FACTOR,roll);
        alSourcef(sources[i],AL_DOPPLER_FACTOR,dopp);
        alSourcef(sources[i],AL_MAX_DISTANCE,dist);
    }
}

void gxAudio::set3dListener( const float pos[3],const float vel[3],const float forward[3],const float up[3] ){
	alListener3f(AL_POSITION,pos[0]*posScale,pos[1]*posScale,pos[2]*posScale);
    float orientation[] = {-forward[0],forward[1],-forward[2],up[0],up[1],up[2]};
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

const float* gxAudio::get3dListenerVel(){
	return listenerVel;
}
