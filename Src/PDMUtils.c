#include <math.h>
#include "PDMUtils.h"
#include "board.h"

/** One or more cycles of sine wave data is generated. This is to minimized the
 *	unwanted distortion when the data is played in cyclical fashion. In other
 *	words output is generated from accurate sine function but with some 
 *	tolerance in the period. Algothm chooses the number of cycles to minimize
 *	this tolerance.
 *
 *  The parameter len is used to indicate the maximum buffer size when calling.
 *  The routine returns the actual number of data stored in the buffer. In
 *  case of error, it returns zero for this parameter.
 *
 *	\param amp amplitude must be greater than zero and smaller than 32768
 *	\param freq frequency
 *	\param srate sample rate
 *	\param buf buffer for the result data
 *	\param len buffer size as an input, data size as an output
 */
void GeneratePcmSine(unsigned amp, int freq, int srate, int16_t *buf, unsigned *len)
{
	double delta;
	int i, count, ncycle, rem, remx;

	// initial setting
	i = ncycle = 1;
	rem = remx = freq;
	delta = (double)srate / freq;

	while(1)
	{
		// data size will exceed the buffer size
		if( delta * i > *len)
		{
			break;
		}

		// compute remainder
		rem = (srate * i) % freq;

		// integer multiple found
		if(rem == 0)
		{
			// no need to continue
			ncycle = i;
			break;
		}
		// new lower remainder found
		else if( rem < remx )
		{
			// replace previous remainder
			remx = rem;
			// and ncycle value as well
			ncycle = i;
		}

		// iteration
		i++;
	}

	// required data count
	count = (int)((double)(srate  * ncycle) / (double)freq + 0.5);
	// check buffer size limitation
	if(count > *len)
	{
		// not enough buffer storage
		*len = 0;
	}
	else
	{
		// check amplitude
		if( amp > 32767)
		{
			amp = 32767;
		}
		// return data size
		*len = count;	
		// time axis resolution
		delta = (2.0 * 3.14159265359 * ncycle) / (double)count;

		for(i = 0; i < count; i++)
		{
			buf[i] = (int16_t)((double)amp * sin((double)i * delta));
		}
	}
}

/** 
 */
void GeneratePdmSine(int freq, int srate, uint16_t *buf, unsigned *len)
{
	int size1, size2;
	int i = 0, k = 0, bit_pos = 15;
	double delta, r = 0., y = 0., d, int_d, prv_d = 0.;

	// total sample count
	size1 = (int)((double)srate / (double)freq + 0.5);
	// required data size
	size2 = (int)((double)size1 / 16. + 0.5);
	// check buffer size
	if(size2 > *len)
	{
		DbgPrintf("\r\n<ERR> not enough buffer for %d", size2);
		// not enough buffer storage
		*len = 0;
	}
	else
	{
		// return data size
		*len = size2;
		// time axis resolution
		delta = 2.0 * 3.14159265359 / (double)size1;

		for(i = 0; i < size1; i++)
		{
			// reference signal
			r = sin((double)i * delta);
			// difference
			d = r - y;
			// integration 1st order
			int_d = d + prv_d;
			// 1 bit quantization
			if(int_d > 0.)
			{
				y = 1.;
				buf[k] = buf[k] + (1<<bit_pos);
			}
			else
			{
				y = -1.;
			}
			// iteration
			prv_d = int_d;
			if(--bit_pos == -1)
			{
				bit_pos = 15;
				k++;
			}
		}
	}
}

/** Delta Sigma modulation
 */
void FilterCIC(uint16_t *inb, uint16_t len, uint16_t dec, int16_t *outb)
{
	int16_t del1 = 0, del2 = 0;
	int16_t sig1 = 0, sig2 = 0, sig3 = 0;
	int16_t pdel1 = 0, pdel2 = 0,  psig3 = 0;

	for(int i = 0; i < len; i++)
	{
		// iteration
		psig3 = sig3;
		pdel1 = del1;
		pdel2 = del2;

		// integrator with decimator
		for(int j = 0; j < 16; j++)
		{
			if( ((inb[i] >> (15-j)) & 0x01) == 0x01 )
			{
				sig1 += 1;
			}
			else
			{
				sig1 -= 1;
			}

			sig2 += sig1;
			sig3 += sig2;
		}

		// comb section
		del1 = sig3 - psig3;
		del2 = del1 - pdel1;

		outb[i] = del2 - pdel2;
	}
}
