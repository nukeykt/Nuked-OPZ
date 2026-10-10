#pragma once

#include <stdint.h>

typedef struct {
    uint32_t subcycle;
    uint32_t cycles;
    uint32_t fsm_cycles;
    uint32_t fsm_cycles_l;

    uint8_t ic;
    uint8_t ic_sync;
    uint8_t fsm_ic_latch;

    // IO
    uint8_t write_data;
    uint8_t write_a_trig;
    uint8_t write_d_trig;
    uint8_t write_a_l;
    uint8_t write_d_l;
    uint8_t write_a_en;
    uint8_t write_d_en;
    uint8_t write_busy;
    uint8_t write_busy_cnt;
    uint8_t mode_address;

    // Noise
    uint32_t noise_lfsr;
    uint32_t noise_timer;
    uint8_t noise_update;
    uint8_t noise_bit;
    uint8_t noise_sync;

    // Register set
    uint8_t mode_test[8];
    uint8_t mode_kon_operator[4];
    uint8_t mode_kon_channel;

    uint8_t reg_address;
    uint8_t reg_address_ready;
    uint8_t reg_data;
    uint8_t reg_data_ready;

    uint8_t noise_freq;

    // Timer
    uint16_t timer_a_reg;
    uint8_t timer_b_reg;
    uint8_t timer_a_temp;
    uint8_t timer_a_do_reset, timer_a_do_load;
    uint8_t timer_a_inc;
    uint16_t timer_a_val;
    uint8_t timer_a_of;
    uint8_t timer_a_load;
    uint8_t timer_a_status;

    uint8_t timer_b_sub;
    uint8_t timer_b_sub_of;
    uint8_t timer_b_inc;
    uint16_t timer_b_val;
    uint8_t timer_b_of;
    uint8_t timer_b_do_reset, timer_b_do_load;
    uint8_t timer_b_temp;
    uint8_t timer_b_status;
    uint8_t timer_irq;

    uint8_t lfo_freq_hi;
    uint8_t lfo_freq_lo;
    uint8_t lfo_pmd;
    uint8_t lfo_amd;
    uint8_t lfo_wave;
    uint8_t lfo2_freq_hi;
    uint8_t lfo2_freq_lo;
    uint8_t lfo2_pmd;
    uint8_t lfo2_amd;
    uint8_t lfo2_wave;

    uint8_t reg_a;
    uint8_t reg_a_trig;
    uint8_t reg_15;
    uint8_t reg_b;
    uint8_t reg_c;
    uint8_t reg_d;
    uint8_t reg_e;
    uint8_t reg_1c;
    uint8_t reg_1e;


    uint8_t reg_kon[4];
    uint32_t reg_counter;
} opz_t;

void OPZ_Clock(opz_t* chip, int32_t* output, uint8_t* sh1, uint8_t* sh2, uint8_t* so);
void OPZ_Write(opz_t* chip, uint32_t port, uint8_t data);
uint8_t OPZ_Read(opz_t* chip, uint32_t port);
uint8_t OPZ_ReadIRQ(opz_t* chip);
uint8_t OPZ_ReadCT1(opz_t* chip);
uint8_t OPZ_ReadCT2(opz_t* chip);
void OPZ_SetIC(opz_t* chip, uint8_t ic);
void OPZ_Reset(opz_t* chip, uint32_t flags);
