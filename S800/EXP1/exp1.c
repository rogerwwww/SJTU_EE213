#include <stdint.h>
#include <stdbool.h>
#include "hw_memmap.h"
#include "debug.h"
#include "gpio.h"
#include "hw_types.h"
#include "pin_map.h"
#include "sysctl.h"

#define   SWITCH_DELAY      (uint32_t)50000
#define   FASTFLASHTIME			(uint32_t)500000
#define   SLOWFLASHTIME			(uint32_t)4000000

uint32_t delay_time,key_value;
uint32_t pressed_counter_0 = 0;
uint32_t pressed_counter_1 = 0;
uint8_t  state_counter = 0;
uint32_t flash_counter_0 = 0;
uint32_t flash_counter_1 = 0;
bool     pressed_flag_0 = false;
bool     pressed_flag_1 = false;
bool     long_pressed_flag_0 = false;
bool     long_pressed_flag_1 = false;

void 		Delay(uint32_t value);
void 		S800_GPIO_Init(void);

int main(void)
{
	S800_GPIO_Init();
	while(1)
  {
		//check SWITCH 1 status. 
		//@output global parameters:
		//  pressed_flag_0        bool  TRUE when the switch is pressed(anti-jitter considered).
		//  long_pressed_flag_0   bool  TRUE when detect a "long press"
		//  state_counter         int   from 0 to 3, represents the current state
		if (GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_0)	== 0)						//USR_SW1-PJ0 pressed
		{
			pressed_flag_0 = true;
			if(++pressed_counter_0 > 50)                              //detected as "long press" when there are more than 50 cycles
			{
				long_pressed_flag_0 = true;			
			}
    }			
		else
		{
			//short press, switch state
			if(pressed_flag_0 && !long_pressed_flag_0)
				if(++state_counter > 3)
					state_counter = 0;
			//reset all globals
			pressed_flag_0 = false;
			long_pressed_flag_0 = false;
			pressed_counter_0 = 0;
		}
		
		//check SWITCH 2 status. 
		//@output global parameters:
		//  pressed_flag_1        bool  TRUE when the switch is pressed(anti-jitter considered).
		//  long_pressed_flag_1   bool  TRUE when detect a "long press"
		if (GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_1)	== 0)          //USR_SW2-PJ1 pressed
		{
			pressed_flag_1 = true;
			if(++pressed_counter_1 > 50)
			{
				long_pressed_flag_1 = true;			
			}
    }			
		else
		{
			//reset all globals
			pressed_flag_1 = false;
			long_pressed_flag_1 = false;
			pressed_counter_1 = 0;
		}
		
		//deal with LED0
		//@input global parameters:
		//  long_pressed_flag_0   bool  TRUE to long lit the LED, FALSE to follow the current state
		//  state_counter         int   from 0 to 3. When state_counter == 0, LED0 flashes; otherwise it is off.
		//@output global parameters:
		//  flash_counter_0       int   from 0 to 40. It will increase itself every cycle, used to control the LED flashing.
    //                              For 0 <= flash_counter <= 20, the LED is on; for 21 <= flash_counter <=40, it's off.	
		if(long_pressed_flag_0)
		{
			 GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0, GPIO_PIN_0);			// Turn on the LED.
		}
		else
  		switch(state_counter)
	  	{
		  	case 0: if(++flash_counter_0 > 20)
				        {
			  	        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0, GPIO_PIN_0);			// Turn on the LED.
				          if(flash_counter_0 > 40)
									  flash_counter_0 = 0;                          //reset flash_counter when exceed the upper limit
							  }
								else
		              GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0, 0x0);							// Turn off the LED.
			          break;
        default: GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0, 0x0);							// Turn off the LED. 
	  	}
		
	  //deal with LED1
		//@input global parameters:
		//  long_pressed_flag_1   bool  TRUE to long lit the LED, FALSE to follow the current state
		//  state_counter         int   from 0 to 3. When state_counter == 2, LED1 flashes; otherwise it is off.
		//@output global parameters:
		//  flash_counter_1       int   from 0 to 40. It will increase itself every cycle, used to control the LED flashing.
    //                              For 0 <= flash_counter <= 20, the LED is on; for 21 <= flash_counter <=40, it's off.	

	  if(long_pressed_flag_1)
		{
			 GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_4, GPIO_PIN_4);			// Turn on the LED.
		}
		else
  		switch(state_counter)
	  	{
		  	case 2: if(++flash_counter_1 > 20)
				        {
									GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_4, GPIO_PIN_4);			// Turn on the LED.									
		           		if(flash_counter_1 > 40)
									  flash_counter_1 = 0;                          //reset flash_counter when exceed the upper limit
							  }
		            else
		              GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_4, 0x0);							// Turn off the LED.
			          break;
        default: GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_4, 0x0);							// Turn off the LED. 
	  	}
			
			
		Delay(SWITCH_DELAY);
	}
}
void Delay(uint32_t value)
{
	uint32_t ui32Loop;
	for(ui32Loop = 0; ui32Loop < value; ui32Loop++){};
}


void S800_GPIO_Init(void)
{
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);						//Enable PortF
	while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF));			//Wait for the GPIO moduleF ready
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOJ);						//Enable PortJ	
	while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOJ));			//Wait for the GPIO moduleJ ready	
	
  GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_0 | GPIO_PIN_4);			//Set PF0&PF4 as Output pin
	GPIOPinTypeGPIOInput(GPIO_PORTJ_BASE,GPIO_PIN_0 | GPIO_PIN_1);        //Set the PJ0,PJ1 as input pin
	GPIOPadConfigSet(GPIO_PORTJ_BASE,GPIO_PIN_0 | GPIO_PIN_1, GPIO_STRENGTH_2MA,GPIO_PIN_TYPE_STD_WPU);
}


