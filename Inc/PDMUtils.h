#ifndef __PDM_UTILS_H
#define __PDM_UTILS_H

#include <stdint.h>

/// Generate sine wave data in 16bit signed integers array
void GeneratePcmSine(unsigned amp, int freq, int srate, int16_t *buf, unsigned *len);
/// Generate sine wave data packed in 16bit unsigned integer array
void GeneratePdmSine(int freq, int srate, uint16_t *buf, unsigned *len);
/// CIC filter
void FilterCIC(uint16_t *inb, uint16_t len, uint16_t dec, int16_t *outb);

#endif // __PDM_UTILS_H
