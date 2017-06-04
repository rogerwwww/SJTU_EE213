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

#define SYSTICK_FREQUENCY    1000      //1000hz

#define  I2C_FLASHTIME        500        //500mS
#define GPIO_FLASHTIME      300        //300mS

#define SUB_PRIORITY        2
//*****************************************************************************
//
//I2C GPIO chip address and resigster define
//
//*****************************************************************************
#define TCA6424_I2CADDR            0x22
#define PCA9557_I2CADDR            0x18

#define PCA9557_INPUT              0x00
#define  PCA9557_OUTPUT            0x01
#define PCA9557_POLINVERT          0x02
#define PCA9557_CONFIG             0x03

#define TCA6424_CONFIG_PORT0       0x0c
#define TCA6424_CONFIG_PORT1       0x0d
#define TCA6424_CONFIG_PORT2       0x0e

#define TCA6424_INPUT_PORT0        0x00
#define TCA6424_INPUT_PORT1        0x01
#define TCA6424_INPUT_PORT2        0x02

#define TCA6424_OUTPUT_PORT0       0x04
#define TCA6424_OUTPUT_PORT1       0x05
#define TCA6424_OUTPUT_PORT2       0x06




void      Delay(uint32_t value);
void      S800_GPIO_Init(void);
uint8_t   I2C0_WriteByte(uint8_t DevAddr, uint8_t RegAddr, uint8_t WriteData);
uint8_t   I2C0_ReadByte(uint8_t DevAddr, uint8_t RegAddr);
void      S800_I2C0_Init(void);
void      S800_UART_Init(void);
void      UARTStringPut(uint8_t *cMessage);

//systick software counter define
volatile uint16_t systick_5ms_couter,systick_100ms_couter;
volatile uint8_t  systick_5ms_status,systick_100ms_status,uart_command_status;

//Task priority
volatile uint8_t task_1_pri = 5 * SUB_PRIORITY, task_2_pri = 5 * SUB_PRIORITY;
volatile uint8_t task_3_pri = 5 * SUB_PRIORITY, mask_pri = 6 * SUB_PRIORITY;

volatile uint8_t result,cnt,key_value,gpio_status;
volatile uint8_t rightshift = 0x01;
uint32_t ui32SysClock,ui32IntPriorityGroup,ui32IntPriorityMask;
uint32_t ui32IntPrioritySystick,ui32IntPriorityUart0;

const uint8_t seg7[] = {0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7d,0x07,0x7f,0x6f,0x77,0x7c,0x58,0x5e,0x079,0x71,0x5c,0x40};
  
uint8_t clock_num[] = {0, 0, 0x11, 0, 0};

volatile uint8_t buffer_pointer = 0;
volatile uint8_t uart_receive_buffer[256];
uint8_t tmpptr;
int main(void)
{
  volatile uint16_t  task_1_flash_cnt, task_1_flash_time = 2, task_2_flash_cnt, task_2_flash_time = 2;
  volatile uint16_t  task_1_uart_cnt, task_1_uart_time = 10, task_2_uart_cnt, task_2_uart_time = 10, task_3_1s_cnt, task_3_125ms_cnt;
  uint8_t gpio_read_result, time_tmp[4], seg_scan = 0x01, seg_cnt = 0;
  int32_t minutes;
  //use internal 16M oscillator, PIOSC
   //ui32SysClock = SysCtlClockFreqSet((SYSCTL_XTAL_16MHZ |SYSCTL_OSC_INT |SYSCTL_USE_OSC), 16000000);    
  //ui32SysClock = SysCtlClockFreqSet((SYSCTL_XTAL_16MHZ |SYSCTL_OSC_INT |SYSCTL_USE_OSC), 8000000);    
  //use external 25M oscillator, MOSC
   //ui32SysClock = SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ |SYSCTL_OSC_MAIN |SYSCTL_USE_OSC), 25000000);    

  //use external 25M oscillator and PLL to 120M
   //ui32SysClock = SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ |SYSCTL_OSC_MAIN | SYSCTL_USE_PLL |SYSCTL_CFG_VCO_480), 120000000);;    
  ui32SysClock = SysCtlClockFreqSet((SYSCTL_XTAL_16MHZ |SYSCTL_OSC_INT | SYSCTL_USE_PLL |SYSCTL_CFG_VCO_480), 20000000);
  
  SysTickPeriodSet(ui32SysClock/SYSTICK_FREQUENCY);     //Systick at 1000Hz frequency
  SysTickEnable();
  SysTickIntEnable();                                    //Enable Systick interrupt

  S800_GPIO_Init();
  S800_I2C0_Init();
  S800_UART_Init();
  
  IntEnable(INT_UART0);
  UARTIntEnable(UART0_BASE, UART_INT_RX | UART_INT_RT);  //Enable UART0 RX,TX interrupt
  IntMasterEnable();    
  ui32IntPriorityMask        = IntPriorityMaskGet();
  IntPriorityGroupingSet(7);                            //Set all priority to pre-emtption priority
  
  IntPrioritySet(INT_UART0,0xE0);                          //Set INT_UART0 to lowest priority
  IntPrioritySet(FAULT_SYSTICK,0x00);                  //Set INT_SYSTICK to highest priority
    
  ui32IntPriorityGroup      = IntPriorityGroupingGet();

  ui32IntPriorityUart0      = IntPriorityGet(INT_UART0);
  ui32IntPrioritySystick    = IntPriorityGet(FAULT_SYSTICK);
  while (1)
  {
    if (systick_100ms_status)
    {
      systick_100ms_status  = 0;
      //TASK 1
      if (task_1_pri < mask_pri)
      {
        if (++task_1_flash_cnt >= task_1_flash_time)
        {
          task_1_flash_cnt = 0;
          gpio_read_result = GPIOPinRead(GPIO_PORTN_BASE, GPIO_PIN_1);
          GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_1, GPIO_PIN_1 ^ gpio_read_result);
        }
        if (++task_1_uart_cnt >= task_1_uart_time)
        {
          task_1_uart_cnt = 0;
          if (!buffer_pointer)
            UARTStringPut((uint8_t *)"TASK1 is running!\r\n");
        }
      }
      //TASK 2
      if (task_2_pri < mask_pri)
      {
        if (++task_2_flash_cnt >= task_2_flash_time)
        {
          task_2_flash_cnt = 0;
          gpio_read_result = GPIOPinRead(GPIO_PORTN_BASE, GPIO_PIN_0);
          GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0, GPIO_PIN_0 ^ gpio_read_result);
        }
        if (++task_2_uart_cnt >= task_2_uart_time)
        {
          task_2_uart_cnt = 0;
          if (!buffer_pointer)
            UARTStringPut((uint8_t *)"TASK2 is running!\r\n");
        }
      }
      //TASK3 CLOCK
      if (task_3_pri < mask_pri)
      {
        if (++task_3_1s_cnt >= 10)
        {
          task_3_1s_cnt = 0;
          clock_num[4] += 1;
          clock_num[3] += clock_num[4] / 10;
          clock_num[1] += clock_num[3] / 6;
          clock_num[0] += clock_num[1] / 10;
          
          clock_num[4] %= 10;
          clock_num[3] %= 6;
          clock_num[1] %= 10;
          clock_num[0] %= 6;
        }
      }
      //GPIO Pin Read
      gpio_read_result = GPIOPinRead(GPIO_PORTJ_BASE, GPIO_PIN_0|GPIO_PIN_1);
      switch ((GPIO_PIN_0 | GPIO_PIN_1) ^ gpio_read_result)
      {
        case (GPIO_PIN_0 | GPIO_PIN_1):
          mask_pri = (task_1_pri > task_2_pri? task_2_pri + (task_2_pri % SUB_PRIORITY) : task_1_pri + (task_1_pri % SUB_PRIORITY));
          if (task_1_pri % SUB_PRIORITY == 0)
            task_1_pri--;
          if (task_2_pri % SUB_PRIORITY == 0)
            task_2_pri--;
          break;
        case (GPIO_PIN_0):
          mask_pri = task_1_pri + (task_1_pri % SUB_PRIORITY);
          if (task_1_pri % SUB_PRIORITY == 0)
            task_1_pri--;
          if (task_2_pri % SUB_PRIORITY != 0)
            task_2_pri++;
          break;
        case (GPIO_PIN_1):
          mask_pri = task_2_pri + (task_2_pri % SUB_PRIORITY);
          if (task_1_pri % SUB_PRIORITY != 0)
            task_1_pri++;
          if (task_2_pri % SUB_PRIORITY == 0)
            task_2_pri--;
          break;
        default:
          if (task_1_pri % SUB_PRIORITY != 0)
            task_1_pri++;
          if (task_2_pri % SUB_PRIORITY != 0)
            task_2_pri++;
          mask_pri = 6 * SUB_PRIORITY;
      }
      if (mask_pri > 6 * SUB_PRIORITY)
        mask_pri = 6 * SUB_PRIORITY;
    }
    if (systick_5ms_status)
    {
      systick_5ms_status = 0;
      //TASK 3 segment display & LED display
      if (task_3_pri < mask_pri)
      {
        result               = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[clock_num[seg_cnt]]);  //write port 1         
        result               = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,seg_scan);  //write port 2
        
        seg_cnt++;
        seg_scan <<= 1;
        if (seg_cnt > 4)
        {
          seg_cnt = 0;
          seg_scan = 0x01;
        }
        if (++task_3_125ms_cnt >= 25)
        {
          task_3_125ms_cnt = 0;
          result = I2C0_WriteByte(PCA9557_I2CADDR,PCA9557_OUTPUT,~rightshift);  

          cnt++;
          rightshift= rightshift<<1;

          if (cnt      >= 0x8)
          {
            rightshift= 0x01;
            cnt       = 0;
          }
        }        
      }
      else
      {
        result               = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0);  //write port 2
      }
    }
    //UART command handler
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
      
      if(!strncmp((char *)uart_receive_buffer, "TASK", 4) && uart_receive_buffer[4] > '0' && uart_receive_buffer[4] < '4')
      {
        if (!strcmp((char *)(uart_receive_buffer + 5), "+START"))
        {
          switch(uart_receive_buffer[4] - '0')
          {
            case 1: task_1_pri = 5 * SUB_PRIORITY; break;
            case 2: task_2_pri = 5 * SUB_PRIORITY; break;
            default: task_3_pri = 5 * SUB_PRIORITY;
          }
        }
        else if(!strcmp((char *)(uart_receive_buffer + 5), "+STOP"))
        {
          switch(uart_receive_buffer[4] - '0')
          {
            case 1: task_1_pri = 7 * SUB_PRIORITY; break;
            case 2: task_2_pri = 7 * SUB_PRIORITY; break;
            default: task_3_pri = 7 * SUB_PRIORITY;
          }
        }
        else if(!strncmp((char *)(uart_receive_buffer + 5), "+FLASH", 6))
        {
          if (!strcmp((char *)(uart_receive_buffer + 11), "200MS"))
          {
            switch(uart_receive_buffer[4] - '0')
            {
              //2 refers to 200 ms here
              case 1: task_1_flash_time = 2; break;
              case 2: task_2_flash_time = 2; break;
              default: goto invalid_command;
            }
          }
          else if (!strcmp((char *)(uart_receive_buffer + 11), "500MS"))
          {
            switch(uart_receive_buffer[4] - '0')
            {
              //5 refers to 500 ms here
              case 1: task_1_flash_time = 5; break;
              case 2: task_2_flash_time = 5; break;
              default: goto invalid_command;
            }
          }
          else if (!strcmp((char *)(uart_receive_buffer + 11), "1000MS"))
          {
            switch(uart_receive_buffer[4] - '0')
            {
              //10 refers to 1000 ms here
              case 1: task_1_flash_time = 10; break;
              case 2: task_2_flash_time = 10; break;
              default: goto invalid_command;
            }
          }
          else if (!strcmp((char *)(uart_receive_buffer + 11), "2000MS"))
          {
            switch(uart_receive_buffer[4] - '0')
            {
              //20 refers to 2000 ms here
              case 1: task_1_flash_time = 20; break;
              case 2: task_2_flash_time = 20; break;
              default: goto invalid_command;
            }
          }
          else
            goto invalid_command;
        }
        else if(!strncmp((char *)(uart_receive_buffer + 5), "+PRIORITY+", 10) 
          && uart_receive_buffer[15] >= '0' && uart_receive_buffer[15] <= '5')
        {
          switch(uart_receive_buffer[4] - '0')
          {
            case 1: task_1_pri = (uart_receive_buffer[15] - '0') * SUB_PRIORITY; break;
            case 2: task_2_pri = (uart_receive_buffer[15] - '0') * SUB_PRIORITY; break;
            default: task_3_pri = (uart_receive_buffer[15] - '0') * SUB_PRIORITY;
          }
        }
        else 
          goto invalid_command;
        
        UARTStringPut((uint8_t *) uart_receive_buffer);
        UARTCharPut(UART0_BASE, '\r');
        UARTCharPut(UART0_BASE, '\n');
      }
      else
        invalid_command: UARTStringPut((uint8_t *)"???\r\n");
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
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);            //Enable PortA
  while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOA));      //Wait for the GPIO moduleA ready

  GPIOPinConfigure(GPIO_PA0_U0RX);                        // Set GPIO A0 and A1 as UART pins.
  GPIOPinConfigure(GPIO_PA1_U0TX);          

  GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

  // Configure the UART for 115,200, 8-N-1 operation.
  UARTConfigSetExpClk(UART0_BASE, ui32SysClock,115200,(UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |UART_CONFIG_PAR_NONE));
  UARTStringPut((uint8_t *)"\r\nHello, world!\r\n");
}
void S800_GPIO_Init(void)
{
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);            //Enable PortF
  while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF));      //Wait for the GPIO moduleF ready
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOJ);            //Enable PortJ  
  while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOJ));      //Wait for the GPIO moduleJ ready  
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPION);            //Enable PortN  
  while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPION));      //Wait for the GPIO moduleN ready    
  
  GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_0);      //Set PF0 as Output pin
  GPIOPinTypeGPIOOutput(GPIO_PORTN_BASE, GPIO_PIN_0);      //Set PN0 as Output pin
  GPIOPinTypeGPIOOutput(GPIO_PORTN_BASE, GPIO_PIN_1);      //Set PN1 as Output pin  

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

  I2CMasterInitExpClk(I2C0_BASE,ui32SysClock, true);                    //config I2C0 400k
  I2CMasterEnable(I2C0_BASE);  

  result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_CONFIG_PORT0,0x0ff);    //config port 0 as input
  result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_CONFIG_PORT1,0x0);      //config port 1 as output
  result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_CONFIG_PORT2,0x0);      //config port 2 as output 

  result = I2C0_WriteByte(PCA9557_I2CADDR,PCA9557_CONFIG,0x00);          //config port as output
  result = I2C0_WriteByte(PCA9557_I2CADDR,PCA9557_OUTPUT,0x0ff);        //turn off the LED1-8
  
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
//  I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_SEND_START);    
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
  if (systick_100ms_couter  != 0)
    systick_100ms_couter--;
  else
  {
    systick_100ms_couter  = SYSTICK_FREQUENCY/10;
    systick_100ms_status   = 1;
  }
  
  if (systick_5ms_couter  != 0)
    systick_5ms_couter--;
  else
  {
    systick_5ms_couter    = SYSTICK_FREQUENCY/200;
    systick_5ms_status   = 1;
  }  
}

/*
  Corresponding to the startup_TM4C129.s vector table UART0_Handler interrupt program name
*/
void UART0_Handler(void)
{
  int32_t uart0_int_status;
  uart0_int_status     = UARTIntStatus(UART0_BASE, true);    // Get the interrrupt status.

  UARTIntClear(UART0_BASE, uart0_int_status);                //Clear the asserted interrupts

  while(UARTCharsAvail(UART0_BASE))                          // Loop while there are characters in the receive FIFO.
  {
    ///Read the next character from the UART and write it back to the UART.
    uart_receive_buffer[buffer_pointer] = UARTCharGetNonBlocking(UART0_BASE);
    UARTCharPutNonBlocking(UART0_BASE,uart_receive_buffer[buffer_pointer]);
    
    // Change CR to CRLF
    if(uart_receive_buffer[buffer_pointer] == '\r')
    {
      //UARTCharPutNonBlocking(UART0_BASE,'\n');
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
    
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0,GPIO_PIN_0 );    
  }
  GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0,0 );  
}
