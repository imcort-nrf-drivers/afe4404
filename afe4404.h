#ifndef __AFE4404_H_
#define __AFE4404_H_

#include "transfer_handler.h"

//LED1 Output Value
#define LED1VAL           0x2C /**< LED1 Output code in twos complement      */

//LED2 Output Value
#define LED2VAL           0x2A /**< LED2 Output code in twos complement      */

//LED3 or Ambient 2 Value
#define LED3VAL           0x2B  /**< LED3 / Ambient 2 value in twos complement*/

//Ambient 1 Value
#define ALED1VAL          0x2D  /**< Ambient 1 value in twos complement       */

//LED1-Ambient 1 Value
#define LED1_ALED1VAL     0x2F  /**< LED1-ambient1 in twos complement         */

//LED2-Ambient 2 Value
#define LED2_ALED2VAL     0x2E  /**< LED2-ambient2 in twos complement         */


#define GAIN_RES_500K    0
#define GAIN_RES_250K    1
#define GAIN_RES_100K    2
#define GAIN_RES_50K     3
#define GAIN_RES_25K   	 4
#define GAIN_RES_10K   	 5
#define GAIN_RES_1M  	 6
#define GAIN_RES_2M   	 7

void afe4404_begin(void);
void afe4404_sleep(void);

void afe4404_setLEDCurrent(uint8_t led, uint8_t current);
void afe4404_setTiaGain(uint8_t led, uint8_t gain_index);
void afe4404_setReverseCurrent(uint8_t led, uint8_t polarity, uint8_t magnitude);

int32_t afe4404_readADC32(uint8_t led_address);

float afe4404_readCurrent(uint8_t reg);

#endif
