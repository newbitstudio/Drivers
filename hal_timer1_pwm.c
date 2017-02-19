/**************************************************************************************************
Filename:       hal_timer1_pwm.c
Editor:         Tome @ newbit
Revised:        $Date: 2017/2/19 11:20:02 +0800  $
Revision:       $Revision: 00001 $

Description:    CC2530 Timer1 , Modulo Mode, User can select which location & channel to operate
                individully

History:        
Notes:          share on github.com/newbitstudio, supports on bbs.newbitstudio.com

**************************************************************************************************/



/**************************************************************************************************
// INCLUDES
**************************************************************************************************/
#include "ioCC2530.h"
#include "hal_defs.h"
#include "hal_types.h"
#include "comdef.h"

#include "hal_timer1_pwm.h"



/**************************************************************************************************
// TYPEDEF
**************************************************************************************************/



/**************************************************************************************************
// CONSTANTS
**************************************************************************************************/




// OPTION

#define PWM_LOCATION_1    0x00    //P02 P03 P04 P05 P06 
#define PWM_LOCATION_2    0x40    //P12 P11 P10 P07 P06


#define T1_CLK_DIV_1      0x00
#define T1_CLK_DIV_8      0x10
#define T1_CLK_DIV_32     0x20
#define T1_CLK_DIV_128    0x30




// register
#define PWM_LACATION_CFG_REG  PERCFG



#if ( HAL_TIMER1_LOC_1 == HAL_TIMER1_LOC_SEL)

#define PWM_LOCATION_VAL  PWM_LOCATION_1

#define T1PWM_PxSEL_CH0   P0SEL
#define T1PWM_PxSEL_CH1   P0SEL
#define T1PWM_PxSEL_CH2   P0SEL
#define T1PWM_PxSEL_CH3   P0SEL
#define T1PWM_PxSEL_CH4   P0SEL

#define T1PWM_BIT_CH0     BV(2)
#define T1PWM_BIT_CH1     BV(3)
#define T1PWM_BIT_CH2     BV(4)
#define T1PWM_BIT_CH3     BV(5)
#define T1PWM_BIT_CH4     BV(6)


#else

#define PWM_LOCATION_VAL  PWM_LOCATION_2

#define T1PWM_PxSEL_CH0   P1SEL
#define T1PWM_PxSEL_CH1   P1SEL
#define T1PWM_PxSEL_CH2   P1SEL
#define T1PWM_PxSEL_CH3   P0SEL
#define T1PWM_PxSEL_CH4   P0SEL

#define T1PWM_BIT_CH0     BV(2)
#define T1PWM_BIT_CH1     BV(1)
#define T1PWM_BIT_CH2     BV(0)
#define T1PWM_BIT_CH3     BV(7)
#define T1PWM_BIT_CH4     BV(6)

#endif



/**************************************************************************************************
// LOCAL VERIABLE
**************************************************************************************************/







/**************************************************************************************************
// FUNCTIONS DECLERATION
**************************************************************************************************/


uint8 halTimer1PWMFreq(uint8 freq);

uint8 halTimer1PWMInit(uint8 freq);
uint8 halTimer1PWMDuty(uint8 ch, uint8 duty);
uint8 halTimer1ChannelEnable(uint8 ch, uint8 duty, uint8 onoff);



/**************************************************************************************************
// FUNCTIONS
**************************************************************************************************/

/**************************************************************************************************
* @fn      halTimer1PWMInit
*
* @brief   
*
* @param   void
*
* @return  uint8 
**************************************************************************************************/
uint8 halTimer1PWMInit(uint8 freq)
{

  PWM_LACATION_CFG_REG &= ~0x40;
  PWM_LACATION_CFG_REG |= PWM_LOCATION_VAL;

  
  
  uint8 res = halTimer1PWMFreq (freq);
  
  if ( res == FAILURE )
    return res;
  
  
  T1CCTL0 |= 0x40;
  T1CTL |= 0x02;  // mode, modulo, start
  
  return res;
  
}



/**************************************************************************************************
* @fn      halTimer1PWMFreq
*
* @brief   
*
* @param   uint8 freq
*
* @return  uint8
**************************************************************************************************/
uint8 halTimer1PWMFreq(uint8 freq)
{
  uint16 t1cc0 = 0;
  uint32 t1freq = 32000000;
  uint8  t1div = 0;
  
  uint8 res = SUCCESS;
  
  /*
    OSAL ÖÐÏµÍ³Ê±ÖÓÎ´·ÖÆµÎª32MHZ
  
    Timer1 Ö§³Ö·ÖÆµÊý   PWM»ù±¾ÆµÂÊ·¶Î§      
    1                   16MHZ - 488.2Hz
    8                   2MHZ  - 61.0Hz
    32                  0.5Mhz  - 15.3Hz
    128                 125Khz -  3.8Hz
  
    µÆ¿ØPWM £¬È¡400HZ,Ô´×ÔÍøÂç
  */
  
  if ( freq <= PWM_BASE_FREQ_HZ_MAX )
  {
    t1div = T1_CLK_DIV_128;
    t1freq /= 128;
  }
  else
  {
    //...
  }
  
  switch ( freq )
  {
  case PWM_BASE_FREQ_HZ_200:
    t1cc0 = (uint16)(t1freq / 200);
    break;
    
    
  case PWM_BASE_FREQ_HZ_400:
    t1cc0 = (uint16)(t1freq / 400);
    break;
    
  default:
    
    res = FAILURE;
    break;
    
  }
  
  
  if ( res == FAILURE)
    return res;
  
  T1CTL &= ~0x30;
  T1CTL |= t1div;
    
  T1CC0L =  LO_UINT16(t1cc0); 
  T1CC0H =  HI_UINT16(t1cc0);
   
  
  return SUCCESS;
}


/**************************************************************************************************
* @fn      halTimer1PWMDuty
*
* @brief   
*
* @param   uint8 ch, uint8 duty
*
* @return  uint8
**************************************************************************************************/
uint8 halTimer1PWMDuty(uint8 ch, uint8 duty)
{
  uint16 t1cc0 = 0;
  uint16 t1ccn = 0;
  
  if ( duty == 0)
    return FAILURE;
  
  if (duty > 100)
    duty = 100;
  
  t1cc0 = T1CC0H;
  t1cc0 <<= 8;
  t1cc0 |= T1CC0L;
  
  uint32 tmp = t1cc0;
  tmp *= duty;
  tmp /= 100;
  
  t1ccn = (uint16)tmp;
  
  switch(ch)
  {
    case HAL_T1PWM_CH1:    
    T1CC1L = LO_UINT16(t1ccn);
    T1CC1H = HI_UINT16(t1ccn);
    break;
    case HAL_T1PWM_CH2:
    T1CC2L = LO_UINT16(t1ccn);
    T1CC2H = HI_UINT16(t1ccn);    
    break;
    case HAL_T1PWM_CH3:
    T1CC3L = LO_UINT16(t1ccn);
    T1CC3H = HI_UINT16(t1ccn);    
    break;
    case HAL_T1PWM_CH4:
    T1CC4L = LO_UINT16(t1ccn);
    T1CC4H = HI_UINT16(t1ccn);
    break;
  }
  
  return SUCCESS;
}



/**************************************************************************************************
* @fn      halTimer1Enable
*
* @brief   
*
* @param   uint8 ch
*
* @return  uint8
**************************************************************************************************/
uint8 halTimer1ChannelEnable(uint8 ch, uint8 duty, uint8 onoff)
{
  if ( onoff == TRUE )
  {

    
    if ( ch & HAL_T1PWM_CH1 )
    {
      T1PWM_PxSEL_CH1 |= T1PWM_BIT_CH1;
      T1CCTL1 &= ~0x3C;
      T1CCTL1 |= 0x18; // Set output on compare-up, clear on 0 
      T1CCTL1 |= 0x04; // Compare mode
    }

    if ( ch & HAL_T1PWM_CH2 )
    {
      T1PWM_PxSEL_CH2 |= T1PWM_BIT_CH2;
      T1CCTL2 &= ~0x3C;
      T1CCTL2 |= 0x18; // Set output on compare-up, clear on 0 
      T1CCTL2 |= 0x04; // Compare mode      
    }
    
    if ( ch & HAL_T1PWM_CH3 )
    {
      T1PWM_PxSEL_CH3 |= T1PWM_BIT_CH3;
      T1CCTL3 &= ~0x3C;
      T1CCTL3 |= 0x18; // Set output on compare-up, clear on 0 
      T1CCTL3 |= 0x04; // Compare mode      
    }
    
    if ( ch & HAL_T1PWM_CH4 )
    {
      T1PWM_PxSEL_CH4 |= T1PWM_BIT_CH4;
      T1CCTL4 &= ~0x3C;
      T1CCTL4 |= 0x18; // Set output on compare-up, clear on 0 
      T1CCTL4 |= 0x04; // Compare mode      
    }
  }
  else
  {

    
    if ( ch & HAL_T1PWM_CH1 )
    {
      T1PWM_PxSEL_CH1 &= ~T1PWM_BIT_CH1;
    }

    if ( ch & HAL_T1PWM_CH2 )
    {
      T1PWM_PxSEL_CH2 &= ~T1PWM_BIT_CH2;
    }
    
    if ( ch & HAL_T1PWM_CH3 )
    {
      T1PWM_PxSEL_CH3 &= ~T1PWM_BIT_CH3;
    }
    
    if ( ch & HAL_T1PWM_CH4 )
    {
      T1PWM_PxSEL_CH4 &= ~T1PWM_BIT_CH4;
    }        
  }
  
  return halTimer1PWMDuty(ch, duty);
}




/**************************************************************************************************
Copyright 2016 Newbit Studio. All rights reserved.
**************************************************************************************************/



