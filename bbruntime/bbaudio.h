
#ifndef BBAUDIO_H
#define BBAUDIO_H

#include "bbsys.h"
#include "../gxruntime/gxaudio.h"
#include "../gxruntime/gxsound.h"

extern gxAudio *gx_audio;

gxSound *	 bbLoadSound( BBStr *file,int use_3d );
gxSound *	 bbStreamSound( BBStr *file,int use_3d );
void		 bbFreeSound( gxSound *sound );
gxChannel *	 bbPlaySound( gxSound *sound,float x,float y,float z,float vx,float vy,float vz );
void		 bbLoopSound( gxSound *sound,int loop );
void		 bbSoundPitch( gxSound *sound,float pitch );
void		 bbSoundVolume( gxSound *sound,float volume );
void		 bbSoundRange( gxSound *sound,float inNear,float inFar );
//void		 bbSoundPan( gxSound *sound,float pan );
void		 bbStopChannel( gxChannel *channel );
void		 bbPauseChannel( gxChannel *channel );
void		 bbResumeChannel( gxChannel *channel );
void		 bbChannelPitch( gxChannel *channel,float pitch );
void		 bbChannelVolume( gxChannel *channel,float volume );
void		 bbChannelRange( gxChannel *channel,float inNear,float inFar );
void		 bbChannelPos( gxChannel *channel,float x,float y,float z,float vx,float vy,float vz );
void         bbChannelSeek( gxChannel *channel,float seconds );
//void		 bbChannelPan( gxChannel *channel,float pan );
int			 bbChannelPlaying( gxChannel *channel );

#endif

