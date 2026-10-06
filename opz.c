#pragma once

#include <stdint.h>

typedef struct {
    uint32_t subcycle;
    uint32_t cycles;
} opz_t;

void OPZ_Clock(opz_t* chip, int32_t* output, uint8_t* sh1, uint8_t* sh2, uint8_t* so);
void OPZ_Write(opz_t* chip, uint32_t port, uint8_t data);
uint8_t OPZ_Read(opz_t* chip, uint32_t port);
uint8_t OPZ_ReadIRQ(opz_t* chip);
uint8_t OPZ_ReadCT1(opz_t* chip);
uint8_t OPZ_ReadCT2(opz_t* chip);
void OPZ_SetIC(opz_t* chip, uint8_t ic);
void OPZ_Reset(opz_t* chip, uint32_t flags);
