/**
 * @file    afe4404.c
 * @author  Simpreative
 * @date    April 2021
 * @brief   Driver interface file for TI AFE4404.
 * @see     https://github.com/imcort-nrf-drivers
 */
 
#include "afe4404.h"

#define AFE4404_I2C_ADDR 0x58

//Default settings. This settings will be used in _begin();

//Internal settings. Not accessibe out of this .c
/******************************************************************************
 *** Register Map
 *****************************************************************************/
//Diagnosis
#define DIAGNOSIS       0x00

//PRPCT ( timer counter )
#define PRPCT            0x1D   /**< Bits 0-15 for writing counter value      */

//Timer Module enable / NUMAV ( # of times to sample and average )
#define TIM_NUMAV        0x1E

#define GAIN_CAP_5PF 		0
#define GAIN_CAP_3PF		1
#define GAIN_CAP_10PF		2
#define GAIN_CAP_8PF		3
#define GAIN_CAP_20PF		4
#define GAIN_CAP_18PF		5
#define GAIN_CAP_25PF		6
#define GAIN_CAP_23PF		7

#define TIA_GAINS1       0x21

//TIA Gains 2
#define TIA_GAINS2       0x20

//LED1 Start / End
#define LED1_ST          0x03
#define LED1_END         0x04

//Sample LED1 Start / End
#define SMPL_LED1_ST     0x07
#define SMPL_LED1_END    0x08

//LED1 Convert Start / End
#define LED1_CONV_ST     0x11
#define LED1_CONV_END    0x12

//Sample Ambient 1 Start / End
#define SMPL_AMB1_ST     0x0B
#define SMPL_AMB1_END    0x0C

//Ambient 1 Convert Start / End
#define AMB1_CONV_ST     0x13
#define AMB1_CONV_END    0x14

//LED2 Start / End
#define LED2_ST          0x09
#define LED2_END         0x0A

//Sample LED2 Start / End
#define SMPL_LED2_ST     0x01
#define SMPL_LED2_END    0x02

//LED2 Convert Start / End
#define LED2_CONV_ST     0x0D
#define LED2_CONV_END    0x0E

//Sample Ambient 2 ( or LED3 ) Start / End
#define SMPL_LED3_ST     0x05
#define SMPL_LED3_END    0x06

//Ambient 2 ( or LED3 ) Convert Start / End
#define LED3_CONV_ST     0x0F
#define LED3_CONV_END    0x10

//ADC Reset Phase 0 Start / End
#define ADC_RST_P0_ST    0x15
#define ADC_RST_P0_END   0x16

//ADC Reset Phase 1 Start / End
#define ADC_RST_P1_ST    0x17
#define ADC_RST_P1_END   0x18

//ADC Reset Phase 2 Start / End
#define ADC_RST_P2_ST    0x19
#define ADC_RST_P2_END   0x1A

//ADC Reset Phase 3 Start / End
#define ADC_RST_P3_ST    0x1B
#define ADC_RST_P3_END   0x1C

//LED Current Control
#define LED_CONFIG       0x22
/* ********************************************************************
 * LED current control is a 24 bit register where                     *
 * LED1: bits 0-5 LED2: bits 6-11 LED3: bits 12-17 and the rest are 0 *
 * ****************************************************************** *
 * LED1, LED2, LED3 Register Values | LED Current Setting (mA)        *
 *                   0              |              0                  *
 *                   1              |             0.8                 *
 *                   2              |             1.6                 *
 *                   3              |             2.4                 *
 *                  ...             |             ...                 *
 *                   63             |              50                 *
 **********************************************************************/


#define SETTINGS          0x23  /**< Settings Address */


//Clockout Settings
#define CLKOUT            0x29  /**< Clockout Address */
/* *****************************************************************************
  *  CLKOUT_DIV Register Settings Graph
  * ****************************************************************************
  * CLKOUT_DIV Register Settings | Division Ratio | Output Clock Freq. ( MHz ) *
  *             0                |         1      |             4              *
  *             1                |         2      |             2              *
  *             2                |         4      |             1              *
  *             3                |         8      |            0.5             *
  *             4                |        16      |           0.25             *
  *             5                |        32      |          0.125             *
  *             6                |        64      |         0.0625             *
  *             7                |       128      |        0.03125             *
  *         8..15                |    Do not use  |      Do not use            *
  *****************************************************************************/

//Diagnostics Flag
#define PD_SHORT_FLAG     0x30  /**< 0: No short across PD 1: Short across PD */

//PD disconnect / INP, INN settings / EXT clock Division settings
#define PD_INP_EXT        0x31
#define PD_DISCONNECT        2  /**< Disconnects PD signals (INP, INM)        */
#define ENABLE_INPUT_SHORT   5  /**< INP, INN are shorted to VCM when TIA dwn */
#define CLKDIV_EXTMODE       0  /**< Ext Clock Div Ration bits 0-2            */

/* *****************************************************************************
  *  CLKDIV_EXTMODE Register Settings Graph
  * ****************************************************************************
  * CLKDIV_EXTMODE Settings      | Division Ratio | Allowed Clock Freq ( MHz ) *
  *             0                |         2      |           8-12             *
  *             1                |         8      |          32-48             *
  *             2                |    Do Not Use  |       Do Not Use           *
  *             3                |         8      |          48-60             *
  *             4                |        16      |          16-24             *
  *             5                |        32      |            4-6             *
  *             6                |        64      |          24-36             *
  *             7                |    Do Not Use  |       Do Not Use           *
  *****************************************************************************/

//PDN_CYCLE Start / End
#define PDNCYCLESTC       0x32  /**< Bits 0-15                                */
#define PDNCYCLEENDC      0x33  /**< Bits 0-15                                */

//Programmable Start / End time for ADC_RDY replacement
#define PROG_TG_STC       0x34  /**< Bits 0-15 Define Start Time              */
#define PROG_TG_ENDC      0x35  /**< Bits 0-15 Define End Time                */

//LED3C Start / End
#define LED3LEDSTC        0x36  /**< LED3 Start, if not used set to 0         */
#define LED3LEDENDC       0x37  /**< LED3 End, if not used set to 0           */

//PRF Clock Division settings
#define CLKDIV_PRF        0x39  /**< Clock Division Ratio for timing engine   */
/* ****************************************************************************
  *   CLKDIV_PRF Register Settings Graph
  * ****************************************************************************
  * CLKDIV_PRF Settings|Division Ratio|Clock Freq(MHz)| Lowest PRF Setting (Hz)*
  *          0         |       1      |       4       |           61           *
  *          1         |  Do Not Use  |  Do Not Use   |      Do Not Use        *
  *          2         |  Do Not Use  |  Do Not Use   |      Do Not Use        *
  *          3         |  Do Not Use  |  Do Not Use   |      Do Not Use        *
  *          4         |       2      |       2       |           31           *
  *          5         |       4      |       1       |           15           *
  *          6         |       8      |     0.5       |            8           *
  *          7         |      16      |    0.25       |            4           *
  *****************************************************************************/

//DAC Settings
#define DAC_SETTING       0x3A  /**< DAC Settings Address                     */
#define POL_OFFDAC_LED2     19  /**< Polarity for LED2                        */
#define I_OFFDAC_LED2       15  /**< Setting for LED2                         */
#define POL_OFFDAC_AMB1     14  /**< Polarity for Ambient 1                   */
#define I_OFFDAC_AMB1       10  /**< Setting for Ambient 1                    */
#define POL_OFFDAC_LED1      9  /**< Polarity for LED1                        */
#define I_OFFDAC_LED1        5  /**< Setting for LED1                         */
#define POL_OFFDAC_LED3      4  /**< Polarity for LED3                        */
#define I_OFFDAC_LED3        0  /**< Setting for LED3                         */
/* **************************************************************
 *  I_OFFDAC Register Settings                                  *
 * **************************************************************
 * * Reg. Settings | Offset Cancellation | Offset Cancellation  |
 * *               |   POL_OFFDAC = 0    |   POL_OFFDAC = 1     |
 * **************************************************************
 *        0        |          0          |           0          *
 *        1        |       0.47          |       –0.47          *
 *        2        |       0.93          |       –0.93          *
 *        3        |        1.4          |        –1.4          *
 *        4        |       1.87          |       –1.87          *
 *        5        |       2.33          |       –2.33          *
 *        6        |        2.8          |        –2.8          *
 *        7        |       3.27          |       –3.27          *
 *        8        |       3.73          |       –3.73          *
 *        9        |        4.2          |        –4.2          *
 *       10        |       4.67          |       –4.67          *
 *       11        |       5.13          |       –5.13          *
 *       12        |        5.6          |        –5.6          *
 *       13        |       6.07          |       –6.07          *
 *       14        |       6.53          |       –6.53          *
 *       15        |          7          |          –7          *
 ****************************************************************/
 
uint8_t TIA_GAIN_PHASE1 = 0;
uint8_t TIA_GAIN_PHASE2 = 0;

uint32_t dac_val = 0;
 
static void afe4404_writeRegister(uint8_t reg_address, uint32_t data)
{
	uint8_t configData[4];
    configData[0] = reg_address;
	configData[1] = (data >> 16) & 0xff;
	configData[2] = (data >> 8) & 0xff;
	configData[3] = data & 0xff;
	
	iic_send(AFE4404_I2C_ADDR, configData, 4, false);
}

static int32_t afe4404_readRegister(uint8_t reg_address)
{
	uint8_t configData[3];
	int32_t retVal;
	
    iic_send(AFE4404_I2C_ADDR, &reg_address, 1, true);
    iic_read(AFE4404_I2C_ADDR, configData, 3);

	retVal = configData[0];
	retVal = (retVal << 8) | configData[1];
	retVal = (retVal << 8) | configData[2];

	if (reg_address >= 0x2A && reg_address <= 0x2F)
	{
		if (retVal & 0x00200000) // check if the ADC value is positive or negative
		{
			retVal &= 0x003FFFFF; // convert it to a 22 bit value
			return (retVal ^ 0xFFC00000);
		}
	}
	return retVal;
}

static int16_t afe4404_readRegister16(uint8_t reg_address)
{
	uint8_t configData[3];
	int32_t retVal;
	
	iic_send(AFE4404_I2C_ADDR, &reg_address, 1, true);
    iic_read(AFE4404_I2C_ADDR, configData, 3);

	retVal = configData[0];
	retVal = (retVal << 8) | configData[1];
	retVal = (retVal << 8) | configData[2];

	return (retVal >> 6) & 0xffff;
}

//Not common used functions. Prepare for begin.
void afe4404_setLEDCurrent(uint8_t led1_current, uint8_t led2_current, uint8_t led3_current)
{
	uint32_t val = 0;
	val |= (led1_current << 0);	 // LED 1 addrss space -> 0-5 bits
	val |= (led2_current << 6);	 // LED 2 addrss space -> 6-11 bits
	val |= (led3_current << 12); // LED 3 addrss space -> 12-17 bits
	afe4404_writeRegister(LED_CONFIG, val);
}

void afe4404_setTiaGain(uint8_t led, uint8_t gain_index)
{
    uint16_t val = 0;
    
    if (gain_index > 7) gain_index = 0;

    val |= (2 << 3); 
    val |= gain_index;

    Debug("Tia Gain Set: LED%d Index: %d", led, gain_index);

    if(led == 1)
    {
        TIA_GAIN_PHASE1 = val & 7;
        afe4404_writeRegister(TIA_GAINS1, val);
    }
    else if(led == 2)
    {
        val |= (1 << 15);
        TIA_GAIN_PHASE2 = val & 7;
        afe4404_writeRegister(TIA_GAINS2, val);
    }
}

void afe4404_setReverseCurrent(uint8_t led, uint8_t polarity, uint8_t magnitude)
{
    if (magnitude > 15) magnitude = 15;
    if (polarity > 1) polarity = 1;

    uint32_t reg_val = magnitude;
    
    if (polarity == 1) {
        reg_val |= 0x10; 
    }

    if(led == 3){ // LED3: bits 0-4
        dac_val &= (~0x1f);
        dac_val |= reg_val;
    } else if(led == 2){ // LED2: bits 15-19
        dac_val &= (~(0x1f << 15));
        dac_val |= (reg_val << 15);
    } else if(led == 1){ // LED1: bits 5-9
        dac_val &= (~(0x1f << 5));
        dac_val |= (reg_val << 5);
    } else if(led == 0){ // Ambient1: bits 10-14
        dac_val &= (~(0x1f << 10));
        dac_val |= (reg_val << 10);
    }

    afe4404_writeRegister(DAC_SETTING, dac_val);
    Debug("Offset DAC Set: LED%d, Pol:%d, Mag:%d, Reg:0x%X", led, polarity, magnitude, dac_val);
}

void afe4404_wakeUp(void)
{
    
    digitalWrite(AFE4404_CS, HIGH);
	
	afe4404_writeRegister(DIAGNOSIS, 0x08);	   //Reset Page 35
	afe4404_writeRegister(SETTINGS, 0x124218); //100mA max LED Current Page 53
	//afe4404_writeRegister(SETTINGS, 0x104218); //50mA max LED Current Page 53

	//Phase Page 26
	afe4404_writeRegister(PRPCT, 7812); //100Hz 39999 512Hz 7812

	//LED2 Not used ambient
	afe4404_writeRegister(LED2_ST, 0);
	afe4404_writeRegister(LED2_END, 0);
	afe4404_writeRegister(SMPL_LED2_ST, 100);
	afe4404_writeRegister(SMPL_LED2_END, 398);
	afe4404_writeRegister(ADC_RST_P0_ST, 5600);
	afe4404_writeRegister(ADC_RST_P0_END, 5606);
	afe4404_writeRegister(LED2_CONV_ST, 5608);
	afe4404_writeRegister(LED2_CONV_END, 6067);

	//Set 0 if LED3 is not used. LED3/Ambient2 IR
	afe4404_writeRegister(LED3LEDSTC, 400);
	afe4404_writeRegister(LED3LEDENDC, 798);

	afe4404_writeRegister(SMPL_LED3_ST, 500);
	afe4404_writeRegister(SMPL_LED3_END, 798);
	afe4404_writeRegister(ADC_RST_P1_ST, 6069);
	afe4404_writeRegister(ADC_RST_P1_END, 6075);
	afe4404_writeRegister(LED3_CONV_ST, 6077);
	afe4404_writeRegister(LED3_CONV_END, 6536);

	//LED1 Red
	afe4404_writeRegister(LED1_ST, 800);
	afe4404_writeRegister(LED1_END, 1198);
	afe4404_writeRegister(SMPL_LED1_ST, 900);
	afe4404_writeRegister(SMPL_LED1_END, 1198);
	afe4404_writeRegister(ADC_RST_P2_ST, 6538);
	afe4404_writeRegister(ADC_RST_P2_END, 6544);
	afe4404_writeRegister(LED1_CONV_ST, 6546);
	afe4404_writeRegister(LED1_CONV_END, 7006);

	//Ambient1
	afe4404_writeRegister(SMPL_AMB1_ST, 1300);
	afe4404_writeRegister(SMPL_AMB1_END, 1598);
	afe4404_writeRegister(ADC_RST_P3_ST, 7008);
	afe4404_writeRegister(ADC_RST_P3_END, 7014);
	afe4404_writeRegister(AMB1_CONV_ST, 7016);
	afe4404_writeRegister(AMB1_CONV_END, 7475);

	//PDNCYCLE
	afe4404_writeRegister(PDNCYCLESTC, 7675);
	afe4404_writeRegister(PDNCYCLEENDC, 7811);

	afe4404_writeRegister(TIM_NUMAV, 0x100 | 3); //ADC Average num 0-15 Page50

	//	clock div 0->4Mhz, 1=2=3 -> do not use, 4-> 2Mhz, 5->1Mhz, 6->0.5Mhz, 7-> 0.25Mhz
	afe4404_writeRegister(CLKDIV_PRF, 0); //CLKDIV Page62

	afe4404_setLEDCurrent(5, 0, 30); // parm1 -> LED1, | parm2 -> LED2, | parm3 -> LED3,    each is 6 bit resolution (0-63)
								 //For epidermal: Red,IR,Null Confirmed
	
	afe4404_setTiaGain(1, GAIN_RES_100K);  //
	afe4404_setTiaGain(2, GAIN_RES_10K); //IR
	
	afe4404_setReverseCurrent(1, 1, 10);//Red
	afe4404_setReverseCurrent(2, 0, 10);

}

void afe4404_begin(void)
{
    //Communication init.
    iic_init();
    
	pinMode(AFE4404_CS, OUTPUT);
	
	afe4404_wakeUp();
	
}

void afe4404_sleep(void)
{
    
    digitalWrite(AFE4404_CS, LOW);

}

int32_t afe4404_readADC32(uint8_t led_address)
{
	return afe4404_readRegister(led_address);
}

float afe4404_readCurrent(uint8_t reg)
{ //micro amps

	int32_t val = afe4404_readADC32(reg);
	float ADC_voltage;
	uint8_t gain_res_val;

	ADC_voltage = (float)val * (2.4f / 65536.0f);

	switch (reg)
	{
	case LED1VAL:
	case ALED1VAL:
		gain_res_val = TIA_GAIN_PHASE1;
		break;
	case LED2VAL:
	case LED3VAL:
		gain_res_val = TIA_GAIN_PHASE2;
		break;
	}
	switch (gain_res_val)
	{
	case GAIN_RES_500K:
		return ADC_voltage / (0.5f * 2.0f);
	case GAIN_RES_250K:
		return ADC_voltage / (0.25f * 2.0f);
	case GAIN_RES_100K:
		return ADC_voltage / (0.1f * 2.0f);
	case GAIN_RES_50K:
		return ADC_voltage / (0.05f * 2.0f);
	case GAIN_RES_25K:
		return ADC_voltage / (0.025f * 2.0f);
	case GAIN_RES_10K:
		return ADC_voltage / (0.01f * 2.0f);
	case GAIN_RES_1M:
		return ADC_voltage / (1.0f * 2.0f);
	case GAIN_RES_2M:
		return ADC_voltage / (2.0f * 2.0f);
	}

	return 0.0f;
}
