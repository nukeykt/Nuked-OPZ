#include <string.h>
#include <stdint.h>
#include "opz.h"

enum {
    eg_num_attack = 0,
    eg_num_decay = 1,
    eg_num_sustain = 2,
    eg_num_release = 3
};

static void OPZ_KeyOn(opz_t* chip) {
    uint32_t b0, b1, b2, b3;

    if (chip->reg_counter == chip->mode_kon_channel) {
        b0 = (chip->reg_kon_operator[1] >> 0) & 1;
        b1 = (chip->reg_kon_operator[1] >> 3) & 1;
        b2 = (chip->reg_kon_operator[1] >> 1) & 1;
        b3 = (chip->reg_kon_operator[1] >> 2) & 1;
    } else {
        b0 = chip->ic ? 0 : (chip->reg_kon[3] >> 7) & 1;
        b1 = (chip->reg_kon[0] >> 7) & 1;
        b2 = (chip->reg_kon[1] >> 7) & 1;
        b3 = (chip->reg_kon[2] >> 7) & 1;
    }

    chip->reg_kon[0] = (chip->reg_kon[0] << 1) | b0;
    chip->reg_kon[1] = (chip->reg_kon[1] << 1) | b1;
    chip->reg_kon[2] = (chip->reg_kon[2] << 1) | b2;
    chip->reg_kon[3] = (chip->reg_kon[3] << 1) | b3;
}

static void OPZ_Noise(opz_t *chip) {
    uint8_t noise_step = chip->ic || chip->noise_update;
    uint8_t bit = 0;
    if (noise_step) {
        if (!chip->ic) {
            uint8_t rst = (chip->noise_lfsr & 0xffff) == 0 && chip->noise_bit == 0;
            uint8_t xr = ((chip->noise_lfsr >> 13) & 1) ^ chip->noise_bit;
            bit = rst | xr;
        }
        chip->noise_bit = (chip->noise_lfsr >> 15) & 1;
    } else {
        bit = chip->noise_lfsr & 1;
    }
    chip->noise_lfsr <<= 1;
    chip->noise_lfsr |= bit;
}

static void OPZ_NoiseTimer(opz_t *chip) {
    uint32_t timer = chip->noise_timer;
    uint32_t of = chip->noise_timer == (chip->noise_freq ^ 31);

    chip->noise_update = of;

    if (chip->ic || (of && chip->noise_sync)) {
        timer = 0;
    } else if (chip->noise_sync) {
        timer = (timer + 1) & 31;
    }

    chip->noise_timer = timer;

    chip->noise_sync = chip->fsm_cycles == 14 || chip->fsm_cycles == 30;
}

static void OPZ_DoIO1(opz_t* chip) {
    chip->write_a_l = chip->write_a_trig;
    chip->write_d_l = chip->write_d_trig;
}

static void OPZ_DoIO2(opz_t* chip) {
    // Busy
    chip->write_busy_cnt += chip->write_busy;
    chip->write_busy = (!(chip->write_busy_cnt >> 5) && chip->write_busy && !chip->ic_sync) | chip->write_d_en;
    chip->write_busy_cnt &= 0x1f;
    if (chip->ic_sync) {
        chip->write_busy_cnt = 0;
    }

    // Write signal check
    chip->write_a_en = chip->write_a_l;
    chip->write_d_en = chip->write_d_l;
    if (chip->write_a_en) {
        chip->write_a_trig = 0;
    }
    if (chip->write_d_en) {
        chip->write_d_trig = 0;
    }
}

static void OPZ_DoRegWrite(opz_t* chip) {
    int i;
    int newm = (chip->reg_15 & 1);
    // Mode write
    if (chip->write_d_en) {
        switch (chip->mode_address)
        {
        case 0x08:
            for (i = 0; i < 4; i++) {
                chip->mode_kon_operator[i] = (chip->write_data >> (i + 3)) & 0x01;
            }
            chip->mode_kon_channel = chip->write_data & 0x07;
            break;
        case 0x09:
            for (i = 0; i < 8; i++) {
                chip->mode_test[i] = (chip->write_data >> i) & 0x01;
            }
            break;
        case 0x0a:
            chip->reg_a = chip->write_data & 15;
            break;
        case 0x0b:
            chip->reg_b = chip->write_data;
            break;
        case 0x0c:
            chip->reg_c = chip->write_data;
            break;
        case 0x0d:
            chip->reg_d = chip->write_data;
            break;
        case 0x0e:
            chip->reg_e = chip->write_data;
            break;
        case 0x0f:
            chip->noise_freq = chip->write_data & 0x1f;
            break;
        case 0x10:
            chip->timer_a_reg &= 0x03;
            chip->timer_a_reg |= chip->write_data << 2;
            break;
        case 0x11:
            chip->timer_a_reg &= 0x3fc;
            chip->timer_a_reg |= chip->write_data & 0x03;
            break;
        case 0x12:
            chip->timer_b_reg = chip->write_data;
            break;
        case 0x14:
            chip->timer_irqb = (chip->write_data >> 3) & 1;
            chip->timer_irqa = (chip->write_data >> 2) & 1;
            chip->timer_resetb = (chip->write_data >> 5) & 1;
            chip->timer_reseta = (chip->write_data >> 4) & 1;
            chip->timer_loadb = (chip->write_data >> 1) & 1;
            chip->timer_loada = (chip->write_data >> 0) & 1;
            break;
        case 0x15:
            chip->reg_15 = chip->write_data & 3;
            break;
        case 0x16:
            if (newm) {
                chip->lfo2_freq_hi = chip->write_data >> 4;
                chip->lfo2_freq_lo = chip->write_data & 0x0f;
                chip->lfo2_frq_update = 1;
            }
            break;
        case 0x17:
            if (newm) {
                if (chip->write_data & 0x80) {
                    chip->lfo2_pmd = chip->write_data & 0x7f;
                }
                else {
                    chip->lfo2_amd = chip->write_data;
                }
            }
            break;
        case 0x18:
            chip->lfo_freq_hi = chip->write_data >> 4;
            chip->lfo_freq_lo = chip->write_data & 0x0f;
            chip->lfo_frq_update = 1;
            break;
        case 0x19:
            if (chip->write_data & 0x80) {
                chip->lfo_pmd = chip->write_data & 0x7f;
            } else {
                chip->lfo_amd = chip->write_data;
            }
            break;
        case 0x1b:
            chip->lfo_wave = chip->write_data & 0x03;
            if (newm) {
                chip->lfo2_wave = (chip->write_data >> 2) & 0x03;
            }
            chip->io_ct1 = (chip->write_data >> 6) & 0x01;
            chip->io_ct2 = chip->write_data >> 7;
            break;
        case 0x1c:
            if (newm) {
                chip->reg_1c = chip->write_data;
            }
            break;
        case 0x1e:
            if (newm) {
                chip->reg_1e = chip->write_data;
            }
            break;
        }
    }

    chip->reg_a_trig = chip->write_d_en && chip->mode_address == 0x0a && (chip->write_data & 128) != 0;

    // Register data write
    chip->reg_data_ready = chip->reg_data_ready && !chip->write_a_en;
    if (chip->reg_address_ready && chip->write_d_en) {
        chip->reg_data = chip->write_data;
        chip->reg_data_ready = 1;
    }

    // Register address write
    chip->reg_address_ready = chip->reg_address_ready && !chip->write_a_en;
    if (chip->write_a_en && ((chip->write_data & 0xe0) != 0 || (chip->write_data & 0xf8) == 0)) {
        chip->reg_address = chip->write_data;
        chip->reg_address_ready = 1;
    }
    if (chip->write_a_en) {
        chip->mode_address = chip->write_data;
    }

    if (chip->fsm_cycles_l == 30) {
        chip->reg_counter = 0;
    } else {
        chip->reg_counter = (chip->reg_counter + 1) & 31;
    }
}

static void OPZ_DoIC(opz_t* chip) {
    if (chip->ic_sync) {
        chip->reg_data = 0;
        chip->reg_address = 0;
    }
}

// clk:
//    __    __    __    __
// __|  |__|  |__|  |__|  |
//
// hclk1:
// _____       _____
//      |_____|     |_____|
//
// hclk2:
//          __          __
// ________|  |________|  |
//

void OPZ_Clock(opz_t* chip, int32_t* output, uint8_t* sh1, uint8_t* sh2, uint8_t* so)
{
    if (!chip->subcycle) {
        OPZ_DoIO1(chip);

        chip->ic_sync = chip->ic;

    } else {

        OPZ_Noise(chip);
        OPZ_NoiseTimer(chip);
        OPZ_KeyOn(chip);
        OPZ_DoRegWrite(chip);
        OPZ_DoIO2(chip);
        OPZ_DoIC(chip);


        chip->fsm_cycles_l = chip->fsm_cycles;
        if (chip->ic && (chip->fsm_ic_latch & 2) == 0) {
            chip->fsm_cycles = 0;
        } else {
            chip->fsm_cycles = (chip->fsm_cycles + 1) & 31;
        }

        chip->cycles = (chip->cycles + 1) & 31;
    }

    chip->fsm_ic_latch <<= 1;
    chip->fsm_ic_latch |= chip->ic;

    chip->subcycle ^= 1;
}

void OPZ_Write(opz_t* chip, uint32_t port, uint8_t data)
{
    chip->write_data = data;
    if (chip->ic)  {
        return;
    }
    if (port & 0x01) {
        chip->write_d_trig = 1;
    } else {
        chip->write_a_trig = 1;
    }
}
