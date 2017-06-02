#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "hw_memmap.h"
#include "debug.h"
#include "gpio.h"
#include "hw_i2c.h"
#include "hw_types.h"
#include "i2c.h"
#include "pin_map.h"
#include "sysctl.h"
#include "systick.h"
#include "interrupt.h"
#include "uart.h"
#include "hw_ints.h"

#define SYSTICK_FREQUENCY		1000			//1000hz

#define	I2C_FLASHTIME				500				//500mS
#define GPIO_FLASHTIME			300				//300mS
//*****************************************************************************
//
//I2C GPIO chip address and resigster define
//
//*****************************************************************************
#define TCA6424_I2CADDR 					0x22
#define PCA9557_I2CADDR						0x18

#define PCA9557_INPUT							0x00
#define	PCA9557_OUTPUT						0x01
#define PCA9557_POLINVERT					0x02
#define PCA9557_CONFIG						0x03

#define TCA6424_CONFIG_PORT0			0x0c
#define TCA6424_CONFIG_PORT1			0x0d
#define TCA6424_CONFIG_PORT2			0x0e

#define TCA6424_INPUT_PORT0				0x00
#define TCA6424_INPUT_PORT1				0x01
#define TCA6424_INPUT_PORT2				0x02

#define TCA6424_OUTPUT_PORT0			0x04
#define TCA6424_OUTPUT_PORT1			0x05
#define TCA6424_OUTPUT_PORT2			0x06




void 		Delay(uint32_t value);
void 		S800_GPIO_Init(void);
uint8_t 	I2C0_WriteByte(uint8_t DevAddr, uint8_t RegAddr, uint8_t WriteData);
uint8_t 	I2C0_ReadByte(uint8_t DevAddr, uint8_t RegAddr);
void		S800_I2C0_Init(void);
void 		S800_UART_Init(void);
void UARTStringPut(uint8_t *cMessage);
//systick software counter define
volatile uint16_t systick_10ms_couter,systick_100ms_couter,systick_1s_couter;
volatile uint8_t	systick_10ms_status,systick_100ms_status,systick_1s_status,uart_command_status;

volatile uint8_t result,cnt,key_value,gpio_status;
volatile uint8_t rightshift = 0x01;
uint32_t ui32SysClock,ui32IntPriorityGroup,ui32IntPriorityMask;
uint32_t ui32IntPrioritySystick,ui32IntPriorityUart0;

const uint8_t seg7[] = {0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7d,0x07,0x7f,0x6f,0x77,0x7c,0x58,0x5e,0x079,0x71,0x5c};
const char* months[] = {"JAN", "FEB", "MAR", "APR", "MAY", "JUN", "JUL", "AUG", "SEP", "OCT", "NOV", "DEC"};
	
uint8_t clock_num[] = {0, 0, 0, 0, 0, 0};

volatile uint8_t buffer_pointer = 0;
volatile uint8_t uart_receive_buffer[256];
uint8_t tmpptr;
int main(void)
{
	volatile uint16_t	i2c_flash_cnt,gpio_flash_cnt;
	uint8_t month_loop_cnt, time_tmp[4];
  int32_t minutes;
	//use internal 16M oscillator, PIOSC
   //ui32SysClock = SysCtlClockFreqSet((SYSCTL_XTAL_16MHZ |SYSCTL_OSC_INT |SYSCTL_USE_OSC), 16000000);		
	//ui32SysClock = SysCtlClockFreqSet((SYSCTL_XTAL_16MHZ |SYSCTL_OSC_INT |SYSCTL_USE_OSC), 8000000);		
	//use external 25M oscillator, MOSC
   //ui32SysClock = SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ |SYSCTL_OSC_MAIN |SYSCTL_USE_OSC), 25000000);		

	//use external 25M oscillator and PLL to 120M
   //ui32SysClock = SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ |SYSCTL_OSC_MAIN | SYSCTL_USE_PLL |SYSCTL_CFG_VCO_480), 120000000);;		
	ui32SysClock = SysCtlClockFreqSet((SYSCTL_XTAL_16MHZ |SYSCTL_OSC_INT | SYSCTL_USE_PLL |SYSCTL_CFG_VCO_480), 20000000);
	
  SysTickPeriodSet(ui32SysClock/SYSTICK_FREQUENCY);
	SysTickEnable();
	SysTickIntEnable();																		//Enable Systick interrupt

	S800_GPIO_Init();
	S800_I2C0_Init();
	S800_UART_Init();
	
	IntEnable(INT_UART0);
  UARTIntEnable(UART0_BASE, UART_INT_RX | UART_INT_RT);	//Enable UART0 RX,TX interrupt
	IntMasterEnable();		
	ui32IntPriorityMask				= IntPriorityMaskGet();
	IntPriorityGroupingSet(7);														//Set all priority to pre-emtption priority
	
	IntPrioritySet(INT_UART0,0xE0);													//Set INT_UART0 to lowest priority
	IntPrioritySet(FAULT_SYSTICK,0x00);									//Set INT_SYSTICK to highest priority
		
	ui32IntPriorityGroup			= IntPriorityGroupingGet();

	ui32IntPriorityUart0			= IntPriorityGet(INT_UART0);
	ui32IntPrioritySystick		= IntPriorityGet(FAULT_SYSTICK);
	while (1)
	{
		if (systick_10ms_status)
		{
			systick_10ms_status		= 0;
			if (++gpio_flash_cnt	>= GPIO_FLASHTIME/10)
			{
				gpio_flash_cnt			= 0;
				if (gpio_status)
					GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0,GPIO_PIN_0 );
				else
					GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0,0);
				gpio_status					= !gpio_status;
			}
		}
		if (systick_100ms_status)
		{
			systick_100ms_status	= 0;
			if (++i2c_flash_cnt		>= I2C_FLASHTIME/100)
			{
				i2c_flash_cnt				= 0;
				result 							= I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[cnt+1]);	//write port 1 				
				result 							= I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,rightshift);	//write port 2
		
				result = I2C0_WriteByte(PCA9557_I2CADDR,PCA9557_OUTPUT,~rightshift);	

				cnt++;
				rightshift= rightshift<<1;

				if (cnt		  >= 0x8)
				{
					rightshift= 0x01;
					cnt 			= 0;
				}

			}
		}
		if (systick_1s_status)
		{
			systick_1s_status = 0;
			clock_num[5] += 1;
			clock_num[4] += clock_num[5] / 10;
			clock_num[3] += clock_num[4] / 6;
			clock_num[2] += clock_num[3] / 10;
			clock_num[1] += clock_num[2] / 6;
			clock_num[0] += clock_num[1] / 10;
			
			clock_num[5] %= 10;
			clock_num[4] %= 6;
			clock_num[3] %= 10;
			clock_num[2] %= 6;
			if (clock_num[0] >= 2 && clock_num[1] >= 4)
			{
			  clock_num[1] = 0;
        clock_num[0] = 0;				
			}
			else
			{
				clock_num[1] %= 10;
			}
		}
		if (uart_command_status)
		{
			uart_command_status = 0;
			uart_receive_buffer[buffer_pointer] = '\0';
			//Change lower case to upper case
			for(tmpptr = 0; tmpptr <= buffer_pointer; tmpptr++)
			{
				if(uart_receive_buffer[tmpptr] >= 'a' && uart_receive_buffer[tmpptr] <= 'z')
					uart_receive_buffer[tmpptr] -= 'a' - 'A';
			}
			if(!strcmp((char *)uart_receive_buffer,"AT+CLASS"))
				UARTStringPut((uint8_t *)"CLASS F1503028\r\n");
			else if(!strcmp((char *)uart_receive_buffer,"AT+STUDENTCODE"))
				UARTStringPut((uint8_t *)"CODE 515021910285\r\n");
			else if(!strncmp((char *)uart_receive_buffer, "SET", 3) 
				&& strlen((char *)uart_receive_buffer) == 11)
			{
				UARTStringPut((uint8_t *)"TIME ");
				UARTCharPutNonBlocking(UART0_BASE, (clock_num[0] = uart_receive_buffer[3] - '0') + '0');
				UARTCharPutNonBlocking(UART0_BASE, (clock_num[1] = uart_receive_buffer[4] - '0') + '0');
				UARTCharPutNonBlocking(UART0_BASE, ':');				
				UARTCharPutNonBlocking(UART0_BASE, (clock_num[2] = uart_receive_buffer[6] - '0') + '0');
				UARTCharPutNonBlocking(UART0_BASE, (clock_num[3] = uart_receive_buffer[7] - '0') + '0');
				UARTCharPutNonBlocking(UART0_BASE, ':');
				UARTCharPutNonBlocking(UART0_BASE, (clock_num[4] = uart_receive_buffer[9] - '0') + '0');
				UARTCharPutNonBlocking(UART0_BASE, (clock_num[5] = uart_receive_buffer[10] - '0') + '0');
				UARTCharPutNonBlocking(UART0_BASE, '\r');				
				UARTCharPutNonBlocking(UART0_BASE, '\n');								
			}
			else if(!strncmp((char *)uart_receive_buffer, "INC", 3) 
				&& strlen((char *)uart_receive_buffer) == 11)
			{
				UARTStringPut((uint8_t *)"TIME ");
				UARTCharPutNonBlocking(UART0_BASE, (clock_num[0] += uart_receive_buffer[3] - '0') + '0');
				UARTCharPutNonBlocking(UART0_BASE, (clock_num[1] += uart_receive_buffer[4] - '0') + '0');
				UARTCharPutNonBlocking(UART0_BASE, ':');				
				UARTCharPutNonBlocking(UART0_BASE, (clock_num[2] += uart_receive_buffer[6] - '0') + '0');
				UARTCharPutNonBlocking(UART0_BASE, (clock_num[3] += uart_receive_buffer[7] - '0') + '0');
				UARTCharPutNonBlocking(UART0_BASE, ':');
				UARTCharPutNonBlocking(UART0_BASE, (clock_num[4] += uart_receive_buffer[9] - '0') + '0');
				UARTCharPutNonBlocking(UART0_BASE, (clock_num[5] += uart_receive_buffer[10] - '0') + '0');
				UARTCharPutNonBlocking(UART0_BASE, '\r');				
				UARTCharPutNonBlocking(UART0_BASE, '\n');								
			}
			else if (!strcmp((char *)uart_receive_buffer, "GETTIME"))
			{
				UARTStringPut((uint8_t *)"TIME ");
				UARTCharPutNonBlocking(UART0_BASE, clock_num[0] + '0');
				UARTCharPutNonBlocking(UART0_BASE, clock_num[1] + '0');
				UARTCharPutNonBlocking(UART0_BASE, ':');				
				UARTCharPutNonBlocking(UART0_BASE, clock_num[2] + '0');
				UARTCharPutNonBlocking(UART0_BASE, clock_num[3] + '0');
				UARTCharPutNonBlocking(UART0_BASE, ':');
				UARTCharPutNonBlocking(UART0_BASE, clock_num[4] + '0');
				UARTCharPutNonBlocking(UART0_BASE, clock_num[5] + '0');
				UARTCharPutNonBlocking(UART0_BASE, '\r');				
				UARTCharPutNonBlocking(UART0_BASE, '\n');	
			}
			else if (strlen((char *)uart_receive_buffer) == 6)
			{
				for (month_loop_cnt = 0; month_loop_cnt < 12; month_loop_cnt++)
					if (!strncmp((char *)uart_receive_buffer, months[month_loop_cnt], 3))
						break;
				
				if (month_loop_cnt != 12)
				{
					if (uart_receive_buffer[3] == '+')
					{
						UARTStringPut((uint8_t *)months[(month_loop_cnt + (uart_receive_buffer[4] - '0') * 10 + uart_receive_buffer[5] - '0') % 12]);				
					  UARTStringPut((uint8_t *)"\r\n");
					}
					else
					{
						UARTStringPut((uint8_t *)months[(108 + month_loop_cnt - (uart_receive_buffer[4] - '0') * 10 - uart_receive_buffer[5] - '0') % 12]);										
				    UARTStringPut((uint8_t *)"\r\n");
					}
			  }
				else
					goto TIMECAL;
			}	
			else
			TIMECAL: if (strlen((char *)uart_receive_buffer) == 11
				  && ((uart_receive_buffer[0] >= (uint8_t)'0' && uart_receive_buffer[0] <= (uint8_t)'9')
					|| (uart_receive_buffer[1] >= (uint8_t)'0' && uart_receive_buffer[1] <= (uint8_t)'9')
					|| (uart_receive_buffer[3] >= (uint8_t)'0' && uart_receive_buffer[3] <= (uint8_t)'9')
					|| (uart_receive_buffer[4] >= (uint8_t)'0' && uart_receive_buffer[4] <= (uint8_t)'9')
					|| (uart_receive_buffer[6] >= (uint8_t)'0' && uart_receive_buffer[6] <= (uint8_t)'9')
					|| (uart_receive_buffer[7] >= (uint8_t)'0' && uart_receive_buffer[7] <= (uint8_t)'9')
					|| (uart_receive_buffer[9] >= (uint8_t)'0' && uart_receive_buffer[9] <= (uint8_t)'9')
					|| (uart_receive_buffer[10] >= (uint8_t)'0' && uart_receive_buffer[10] <= (uint8_t)'9')))
			{
				if (uart_receive_buffer[5] == '+')
				{
					time_tmp[3] = uart_receive_buffer[4] - '0' * 2 + uart_receive_buffer[10];
					time_tmp[2] = uart_receive_buffer[3] - '0' * 2 + uart_receive_buffer[9] + time_tmp[3] / 10;				
					time_tmp[1] = uart_receive_buffer[1] - '0' * 2 + uart_receive_buffer[7] + time_tmp[2] / 6;
					time_tmp[0] = uart_receive_buffer[0] - '0' * 2 + uart_receive_buffer[6] + time_tmp[1] / 10;
					
					//time_tmp[0] is used here as a temp variable
					time_tmp[0] = (time_tmp[0] * 10 + time_tmp[1]) % 24;
					
					time_tmp[1] = time_tmp[0] % 10;
					time_tmp[0] /= 10;
					time_tmp[2] %= 6;
					time_tmp[3] %= 10;
				}
				else
				{
					minutes = 0;
					minutes += uart_receive_buffer[4] - uart_receive_buffer[10];
					minutes += (uart_receive_buffer[3] - uart_receive_buffer[9]) * 10;				
					minutes += (uart_receive_buffer[1] - uart_receive_buffer[7]) * 60;
					minutes += (uart_receive_buffer[0] - uart_receive_buffer[6]) * 600;
          
					time_tmp[0] = ((minutes / 60) % 24) / 10;
					time_tmp[1] = ((minutes / 60) % 24) % 10;
					minutes %= 60;
					time_tmp[2] = minutes / 10;
					minutes %= 10;
					time_tmp[3] = minutes;
				}

			  UARTCharPutNonBlocking(UART0_BASE, time_tmp[0] + '0');
				UARTCharPutNonBlocking(UART0_BASE, time_tmp[1] + '0');
				UARTCharPutNonBlocking(UART0_BASE, ':');
				UARTCharPutNonBlocking(UART0_BASE, time_tmp[2] + '0');
				UARTCharPutNonBlocking(UART0_BASE, time_tmp[3] + '0');

				UARTCharPutNonBlocking(UART0_BASE, '\r');
				UARTCharPutNonBlocking(UART0_BASE, '\n');

			}
			else
				UARTStringPut((uint8_t *)"???\r\n");
			buffer_pointer = 0;
		}
	}
}

void Delay(uint32_t value)
{
	uint32_t ui32Loop;
	for(ui32Loop = 0; ui32Loop < value; ui32Loop++){};
}


void UARTStringPut(uint8_t *cMessage)
{
	while(*cMessage!='\0')
		UARTCharPut(UART0_BASE,*(cMessage++));
}
void UARTStringPutNonBlocking(const char *cMessage)
{
	while(*cMessage!='\0')
		UARTCharPutNonBlocking(UART0_BASE,*(cMessage++));
}

void S800_UART_Init(void)
{
	SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);						//Enable PortA
	while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOA));			//Wait for the GPIO moduleA ready

	GPIOPinConfigure(GPIO_PA0_U0RX);												// Set GPIO A0 and A1 as UART pins.
  GPIOPinConfigure(GPIO_PA1_U0TX);    			

  GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

	// Configure the UART for 115,200, 8-N-1 operation.
  UARTConfigSetExpClk(UART0_BASE, ui32SysClock,115200,(UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |UART_CONFIG_PAR_NONE));
	UARTStringPut((uint8_t *)"\r\nHello, world!\r\n");
}
void S800_GPIO_Init(void)
{
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);						//Enable PortF
	while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF));			//Wait for the GPIO moduleF ready
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOJ);						//Enable PortJ	
	while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOJ));			//Wait for the GPIO moduleJ ready	
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPION);						//Enable PortN	
	while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPION));			//Wait for the GPIO moduleN ready		
	
  GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_0);			//Set PF0 as Output pin
  GPIOPinTypeGPIOOutput(GPIO_PORTN_BASE, GPIO_PIN_0);			//Set PN0 as Output pin
  GPIOPinTypeGPIOOutput(GPIO_PORTN_BASE, GPIO_PIN_1);			//Set PN1 as Output pin	

	GPIOPinTypeGPIOInput(GPIO_PORTJ_BASE,GPIO_PIN_0 | GPIO_PIN_1);//Set the PJ0,PJ1 as input pin
	GPIOPadConfigSet(GPIO_PORTJ_BASE,GPIO_PIN_0 | GPIO_PIN_1,GPIO_STRENGTH_2MA,GPIO_PIN_TYPE_STD_WPU);
}

void S800_I2C0_Init(void)
{
	uint8_t result;
  SysCtlPeripheralEnable(SYSCTL_PERIPH_I2C0);
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
	GPIOPinConfigure(GPIO_PB2_I2C0SCL);
  GPIOPinConfigure(GPIO_PB3_I2C0SDA);
  GPIOPinTypeI2CSCL(GPIO_PORTB_BASE, GPIO_PIN_2);
  GPIOPinTypeI2C(GPIO_PORTB_BASE, GPIO_PIN_3);

	I2CMasterInitExpClk(I2C0_BASE,ui32SysClock, true);										//config I2C0 400k
	I2CMasterEnable(I2C0_BASE);	

	result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_CONFIG_PORT0,0x0ff);		//config port 0 as input
	result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_CONFIG_PORT1,0x0);			//config port 1 as output
	result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_CONFIG_PORT2,0x0);			//config port 2 as output 

	result = I2C0_WriteByte(PCA9557_I2CADDR,PCA9557_CONFIG,0x00);					//config port as output
	result = I2C0_WriteByte(PCA9557_I2CADDR,PCA9557_OUTPUT,0x0ff);				//turn off the LED1-8
	
}


uint8_t I2C0_WriteByte(uint8_t DevAddr, uint8_t RegAddr, uint8_t WriteData)
{
	uint8_t rop;
	while(I2CMasterBusy(I2C0_BASE)){};
	I2CMasterSlaveAddrSet(I2C0_BASE, DevAddr, false);
	I2CMasterDataPut(I2C0_BASE, RegAddr);
	I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_SEND_START);
	while(I2CMasterBusy(I2C0_BASE)){};
	rop = (uint8_t)I2CMasterErr(I2C0_BASE);

	I2CMasterDataPut(I2C0_BASE, WriteData);
	I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_SEND_FINISH);
	while(I2CMasterBusy(I2C0_BASE)){};

	rop = (uint8_t)I2CMasterErr(I2C0_BASE);
	return rop;
}

uint8_t I2C0_ReadByte(uint8_t DevAddr, uint8_t RegAddr)
{
	uint8_t value,rop;
	while(I2CMasterBusy(I2C0_BASE)){};	
	I2CMasterSlaveAddrSet(I2C0_BASE, DevAddr, false);
	I2CMasterDataPut(I2C0_BASE, RegAddr);
//	I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_SEND_START);		
	I2CMasterControl(I2C0_BASE,I2C_MASTER_CMD_SINGLE_SEND);
	while(I2CMasterBusBusy(I2C0_BASE));
	rop = (uint8_t)I2CMasterErr(I2C0_BASE);
	Delay(1);
	//receive data
	I2CMasterSlaveAddrSet(I2C0_BASE, DevAddr, true);
	I2CMasterControl(I2C0_BASE,I2C_MASTER_CMD_SINGLE_RECEIVE);
	while(I2CMasterBusBusy(I2C0_BASE));
	value=I2CMasterDataGet(I2C0_BASE);
		Delay(1);
	return value;
}

/*
	Corresponding to the startup_TM4C129.s vector table systick interrupt program name
*/
void SysTick_Handler(void)
{
	if (systick_100ms_couter	!= 0)
		systick_100ms_couter--;
	else
	{
		systick_100ms_couter	= SYSTICK_FREQUENCY/10;
		systick_100ms_status 	= 1;
	}
	
	if (systick_10ms_couter	!= 0)
		systick_10ms_couter--;
	else
	{
		systick_10ms_couter		= SYSTICK_FREQUENCY/100;
		systick_10ms_status 	= 1;
	}
	
	if (systick_1s_couter	!= 0)
		systick_1s_couter--;
	else
	{
		systick_1s_couter		= SYSTICK_FREQUENCY/1;
		systick_1s_status 	= 1;
	}
	
	if (GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_0) == 0)
	{
		systick_100ms_status	= systick_10ms_status = 0;
		GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0,GPIO_PIN_0);		
	}
	else
		GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0,0);		
}

/*
	Corresponding to the startup_TM4C129.s vector table UART0_Handler interrupt program name
*/
void UART0_Handler(void)
{
	int32_t uart0_int_status;
  uart0_int_status 		= UARTIntStatus(UART0_BASE, true);		// Get the interrrupt status.

  UARTIntClear(UART0_BASE, uart0_int_status);								//Clear the asserted interrupts

	while(UARTCharsAvail(UART0_BASE))    											// Loop while there are characters in the receive FIFO.
  {
		///Read the next character from the UART and write it back to the UART.
		uart_receive_buffer[buffer_pointer] = UARTCharGetNonBlocking(UART0_BASE);
    UARTCharPutNonBlocking(UART0_BASE,uart_receive_buffer[buffer_pointer]);
		
		// Change CR to CRLF
		if(uart_receive_buffer[buffer_pointer] == '\r')
		{
	    UARTCharPutNonBlocking(UART0_BASE,'\n');
			uart_command_status = 1;
		}
		// Respond to DEL
		else if(uart_receive_buffer[buffer_pointer] == 127)
		{
			if(buffer_pointer)
		    buffer_pointer--;
		}
		//ignore the control symbols
		else if(uart_receive_buffer[buffer_pointer] > 31)
			buffer_pointer++;
		
		GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_1,GPIO_PIN_1 );		
		//Delay(10000000);
	}
	GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_1,0 );	
	
	while (GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_1) == 0)
		GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_1,GPIO_PIN_1);	

	GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_1,0);
}
