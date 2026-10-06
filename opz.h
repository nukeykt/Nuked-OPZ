#include <string.h>
#include <stdint.h>
#include "opz.h"

enum {
    eg_num_attack = 0,
    eg_num_decay = 1,
    eg_num_sustain = 2,
    eg_num_release = 3
};

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


        chip->cycles = (chip->cycles + 1) & 31;
    } else {
    }

    chip->subcycle ^= 1;
}
