#ifndef _SGM53x8_H_
#define _SGM53x8_H_

#include <stdint.h>

void DriverSPIRegWrite(uint16_t byte);
void DriverSPIConfig(void);
void DriverSGM5348Init(void);
void DriverSGM5348SetDac(uint8_t channel,uint16_t Dac_Value);

#endif
