// sndintrf.h: SEP00-Gandalf
#ifndef SNDINTRF_H
#define SNDINTRF_H

int sound_start(void);
void sound_stop(void);
void sound_update(void);
int sound_scalebufferpos(int value);



// GN: from Mame36.
// Not an interface used with Mame27, but glue code for use by mixer.c
int osd_start_audio_stream(int stereo);
//void osd_stop_audio_stream(void);
int osd_update_audio_stream(INT16* buffer);

#endif

