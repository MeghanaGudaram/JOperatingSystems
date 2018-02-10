#include <kern/time.h>
#include <inc/assert.h>
#include <kern/cpu.h>

static unsigned int ticks[NCPU];

void
time_init(void)
{
	for(int i=0; i < NCPU; i++)
	{
		ticks[i] = 0;
	}
}

// This should be called once per timer interrupt.  A timer interrupt
// fires every 10 ms.
void
time_tick(void)
{
	uint32_t cpu_num=cpunum();
	ticks[cpu_num]++;
	if (ticks[cpu_num] * 10 < ticks[cpu_num])
		panic("time_tick: time overflowed");
}

unsigned int
time_msec(void)
{
	return ticks[cpunum()] * 10;
}
