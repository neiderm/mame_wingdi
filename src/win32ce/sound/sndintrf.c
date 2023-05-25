// sndintrf.c: routines scabbed from Mame36 sound engine
// AUG00 - Gandalf
#include "driver.h"
#include "samples.h"
#include "mixer.h"



int sound_start(void)
{
	// from osd_init()
	if (mixer_sh_start() != 0)
		return 1;
/*
	if (streams_sh_start() != 0)
		return 1;
*/

	// loop through the applicable sound interfaces

//	namco_sh_start();
	samples_sh_start();

	return 0;
}


void sound_stop(void)
{
	mixer_sh_stop();
}



void sound_update(void)
{
/*
	while (Machine->drv->sound[totalsound].sound_type != 0 && totalsound < MAX_SOUND)
	{
		if (sndintf[Machine->drv->sound[totalsound].sound_type].update)
			(*sndintf[Machine->drv->sound[totalsound].sound_type].update)();

		totalsound++;
	}
*/

	// GN: called from updatescreen()
//	streams_sh_update();
	mixer_sh_update();
}


int sound_scalebufferpos(int value)
{
	return 0; // GN
/*
	int result = (int)((double)value * timer_timeelapsed (sound_update_timer) * refresh_period_inv);
	if (value >= 0) return (result < value) ? result : value;
	else return (result > value) ? result : value;
*/
}
