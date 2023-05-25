// samples.c
// AUG00 - taken from Mame36, modified by Gandalf to integrate with Mame21

#include "mame.h" // error_log

#include "mixer.h"

#include "driver.h"


extern int g_Samplerate;


/* static */ int firstchannel,numchannels;


// GN: modified interface for sample_start()
/* Start one of the samples loaded from disk. Note: channel must be in the range */
/* 0 .. Samplesinterface->channels-1. It is NOT the discrete channel to pass to */
/* mixer_play_sample() */
void sample_start2(int channel,unsigned char *data,int len,int freq,int volume,int loop)
{
	if (channel >= numchannels)
	{
		if (errorlog) fprintf(errorlog,"error: sample_start() called with channel = %d, but only %d channels allocated\n",channel,numchannels);
		return;
	}

	mixer_play_sample(firstchannel + channel,
				data,
				len,
				freq,
				loop);

}

void sample_set_freq(int channel,int freq)
{
	if (g_Samplerate == 0) return;

	if (channel >= numchannels)
	{
		if (errorlog) fprintf(errorlog,"error: sample_adjust() called with channel = %d, but only %d channels allocated\n",channel,numchannels);
                return;
	}

	mixer_set_sample_frequency(channel + firstchannel,freq);
}

void sample_set_volume(int channel,int volume)
{
	if (g_Samplerate == 0)
        return;

	if (channel >= numchannels)
	{
		if (errorlog) fprintf(errorlog,"error: sample_adjust() called with channel = %d, but only %d channels allocated\n",channel,numchannels);
		return;
	}

	mixer_set_volume(channel + firstchannel,volume * 100 / 255);
}

void sample_stop(int channel)
{
	if (g_Samplerate == 0)
        return;

	if (channel >= numchannels)
	{
		if (errorlog) fprintf(errorlog,"error: sample_stop() called with channel = %d, but only %d channels allocated\n",channel,numchannels);
                return;
	}

	mixer_stop_sample(channel + firstchannel);
}

int sample_playing(int channel)
{
	if (g_Samplerate == 0) return 0;
	if (channel >= numchannels)
	{
		if (errorlog) fprintf(errorlog,"error: sample_playing() called with channel = %d, but only %d channels allocated\n",channel,numchannels);
		return 0;
	}

	return mixer_is_sample_playing(channel + firstchannel);
}

//#define NUMVOICES 8 // GN: the highest number that I know of, Galaga, plays a sample on channel 7

int samples_sh_start(void)
{
	int i;
	int vol[MIXER_MAX_CHANNELS];

	/* read audio samples if available */
// 	Machine->samples = readsamples(intf->samplenames,Machine->gamedrv->name);

	numchannels = NUMVOICES; // 1; // GN // intf->channels;
	for (i = 0;i < numchannels;i++)
		vol[i] = 50; // GN // intf->volume;
	firstchannel = mixer_allocate_channels(numchannels,vol);
	for (i = 0;i < numchannels;i++)
	{
		char buf[40];

		sprintf(buf,"Sample #%d",i);
		mixer_set_name(firstchannel + i,buf);
	}
	return 0;
}
