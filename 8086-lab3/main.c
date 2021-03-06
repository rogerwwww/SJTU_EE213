/* Main.c file generated by New Project wizard
 *
 * Created:   Wed Apr 12 2017
 * Processor: 8086
 * Compiler:  Digital Mars C
 * Author:    Roger Wang
 *
 * Modified from original lab3 assembly file.
 *
 * This is an example for practicing C programming for 8086 in Proteus simulation. 
 * C programming owns a great advantage in programming efficiency over assembly.
 * This program realizes most(but not all) functions in SJTU EE213 8086 lab 3.
 * It should display number "1234" on the seg-7, and by pressing the INT-Test button,
 * the numbers will shift leftward.
 *
 * PLEASE NOTE that you should switch your compiler from MASM32 to Digital Mars C before
 * simulation. Proteus will automatically generate a rtl.asm file, just leave it as it is.
 *
 * For tutorials and issues, visit the github page: https://github.com/rogerwwww/SJTU_EE213
 *
 * Before starting simulation set Internal Memory Size 
 * in the 8086 model properties to 0x10000
 */
#include "header.h"
const static unsigned char SEGTAB[16]={
   3FH,	// 7-Segment Tube, 共阴极类型的7段数码管示意图
   06H,
   5BH,	//            a a a
   4FH,	//         f         b
   66H,	//         f         b
   6DH, //         f         b
   7DH, //            g g g 
   07H, //         e         c
   7FH,	//         e         c
   6FH,	//         e         c
   77H,	//            d d d     h h h
   7CH,	// ----------------------------------
   39H,	//       b7 b6 b5 b4 b3 b2 b1 b0
   5EH,	//       DP  g  f  e  d  c  b  a
   79H,	//
   71H
};

// The watchdog period, actual period = watchdog_period * 2ms
// 看门狗周期，实际周期为（watchdog_period * 2）ms
const static unsigned int watchdog_period = 15;

// Student number for interrupt vector table initialization(in lab-3 assignment)
// 学号，用于初始化中断向量表（lab3中需要实现的功能）
static unsigned char studentNo = 5Dh;

// numbers to be displayed
// 送显示的数字
unsigned int toDisp[4] = {1, 2, 3, 4};

// program entrance
// 主程序入口
void main(void)
{
   //service initiate
   INT_INIT(); //interrupt initiate
   INIT8255(); //initiate 8255
   INIT8253(10000, 0, 3); //set 8253 timer 0, with period 10ms(see schematic for hardware connection)
   INIT8253(100, 1, 3); //set 8253 timer 1, with period 1s
   INIT8253(watchdog_period, 2, 0); //set timer 2 as watchdog
   
   __asm{STI} //enable maskable interrupt
   
   //endless loop
	while (1)
	{
		//display LEDs
		for(int i = 0; i < 4; i++)
		{
			DISPLAY8255(i, toDisp[i]);
			DELAY(40);
		}
		AccessPC();//access 8255 port C
		FeedDog(watchdog_period);//feed watchdog
	}
}
 
// Function name: io_write_word
// Write a word(outword) to the target address(addr). Written in asm.
// Inputs:
//		addr, type word, the target address for output data
//		outword, type word(two bytes), the data to be written
// Output: None
// 函数名称： io_write_word
// 向目标地址（addr）写入一个字长度的数据（outword），汇编实现
// 输入：
//		addr，字类型，输出数据的目标地址
//		outword，字类型，输出数据
// 输出：无
void io_write_word(unsigned int addr, unsigned int outword)
{
	__asm
	{
		MOV DX, addr
		MOV AX, outword
		OUT DX, AX
	}
}

// Function name: io_write_byte
// Write a byte(outbyte) to the target address(addr). Written in asm.
// Inputs:
//		addr, type word, the target address for output data
//		outword, type byte, the data to be written
// Output: None
// 函数名称： io_write_byte
// 向目标地址（addr）写入一个字节长度的数据（outbyte），汇编实现
// 输入：
//		addr，字类型，输出数据的目标地址
//		outword，字节类型，输出数据
// 输出：无
void io_write_byte(unsigned int addr, unsigned char outbyte)
{
	__asm
	{
		MOV DX, addr
		MOV AL, outbyte
		OUT DX, AL
	}
}

// Function name: io_read
// Read a word from the target address(addr). Written in asm.
// The returned value could be assigned to a byte type, taking only lower bits.
// Input:
//		addr, type word, the target address to read data from
// Output: 
//		type word, the data read from the target address
// 函数名称： io_read
// 从目标地址（addr）读取一个字长度的数据，汇编实现
// 返回值可以直接赋值给字节类型的变量。若如此做，只有低八位被读取
// 输入：
//		addr，字类型，读取数据的目标地址
// 输出：
//		字类型，读取到的数据
unsigned int io_read(unsigned int addr)
{
	unsigned int inword;
	__asm
	{
		MOV DX, addr
		IN AX, DX
		MOV inword, AX
	}
	return inword;
}

// Function name: INIT8255
// Initiate 8255 I/O chip. Write the control word by yourself in main.c.
// Please refer to your textbooks, slices or other references.
// Input/Output: None
// 函数名称： INIT8255
// 初始化8255I/O芯片。控制字请在main.c文件的函数实现中修改。
// 如有疑问，请参阅书本/讲义/其他资料
// 输入/输出：无
// 8255 control register notes:
//	D7	1-I/O mode	0-BSR mode
//	FOR I/O MODE(D7 = 1)
//	D6D5	00-Mode 0, 01-Mode 1, 1*-Mode2
//	D4		1-PortA input, 0-PortA output
//	D3		1-PCU input, 0-PCU output
//	D2		0-Mode 0, 1-Mode 1
//	D1		1-PortB input, 0-PortB output
//	D0		1-PCL input, 0-PCL output
void INIT8255(void)
{
	io_write_byte(L8255CS, 0x81);
}

// Function name: INIT8253
// Initiate 8253 timer chip with desired count down number, timer and mode.
// Note: all count down numbers are HEX rather than BCD by default.
// Inputs:
//		count_time, type word, the count down number
//		timer, type word, the selected timer(range 0 to 2)
//		mode, type word, the selected mode(range 0 to 5)
// Output: None
// 函数名称： INIT8253
// 使用设定的计数、计时器和模式初始化8253计时器芯片
// 注意：默认所有的倒计时数字采用十六进制
// 输入：
//		count_time，字类型，倒计时计数
//		timer，字类型，选中的计时器（取值0-2）
//		mode，字类型，选中的模式（取值0-5）
// 输出：无
void INIT8253(unsigned int count_time, unsigned int timer, unsigned int mode)
{
	unsigned int target_addr;
	unsigned char control_word;
	switch(timer)
	{
		case 0:
		target_addr = L8253T0;
		control_word = 0x30;
		break;
		case 1:
		target_addr = L8253T1;
		control_word = 0x70;
		break;
		case 2:
		target_addr = L8253T2;
		control_word = 0xB0;
		break;
		default: return;
	}
	mode *= 2;
	control_word += mode;
	io_write_word(L8253CS, control_word);
	io_write_word(target_addr, count_time);
	count_time >>= 8;
	io_write_word(target_addr, count_time);
}

// Function name: AccessPC
// Read data from 8255 PortC, do some work and write data back to 8255 PortC.
// Input/Output: None
// 函数名称：AccessPC
// 从8255的端口C读取数据，处理后送回端口C
// 输入/输出：无
void AccessPC(void)
{
	unsigned char buffer;
	buffer = io_read(L8255PC);
	if(buffer & 1)
		buffer &= 0xBF;
	else
		buffer |= 0x40;
	io_write_byte(L8255PC, buffer);
}

// Function name: DISPLAY8255
// Send numbers to seg-7 LEDs connected to 8255
// Inputs:
//		segment, type word, the LED chosen(range 0 to 3)
//		displayNum, type byte, the hex number to be displayed
// Output: None
// 函数名称：DISPLAY8255
// 将数据送至连接到8255芯片的七段显示器
// 输入：
//		segment, 字类型，选择的LED（取值0-3）
//		displayNum, 字节类型，被显示的十六进制数
// 输出：无
void DISPLAY8255(unsigned int segment, unsigned char displayNum)
{
	unsigned char toPA;
	switch(segment)
	{
		case 0: toPA = 0xFE; break;
		case 1: toPA = 0xFD; break;
		case 2: toPA = 0xFB; break;
		case 3: toPA = 0xF7; break;
		default: return;
	}
	
	io_write_byte(L8255PA, toPA);
	io_write_byte(L8255PB, SEGTAB[displayNum]);	
}

// Function name: DELAY
// Software delay for a certain cycle of time.
// Input:
//		delayCycle, type word, the number of cycles for delay.
// Output: None
// 函数名称： DELAY
// 持续特定周期的软件延时
// 输入：
//		delayCycle，字类型，延时的周期
// 输出：无
void DELAY(unsigned int delayCycle)
{
   __asm
   {
    	PUSH CX
    	MOV CX, delayCycle
D1: 	LOOP D1
    	POP CX
   }
}

// Function name: INT_INIT
// Initiate interrupt vector table.
// Input/Output: None
// 函数名称：INT_INIT
// 初始化中断矢量表
// 输入/输出：无
void INT_INIT(void)
{
   __asm
   {
		MOV AX, 0
		MOV ES, AX				// Get ready to operate IVT
		
		MOV AX, OFFSET MYIRQ	//! Incorrect if using LEA! dont know why
		
		MOV BL, studentNo		//calculate interrupt offset
		MOV BH, 0
		SHL BX, 1
		SHL BX, 1
		MOV ES:[BX], AX			//write interrupt service address into memory, IP first
		MOV AX, CS
		INC BX
		INC BX
		MOV ES:[BX], AX			//then CS
   }
}

// Function name: MYIRQ
// Interrupt service routine
// Input/Output: None
// 函数名称：MYIRQ
// 中断服务程序
// 输入/输出：无
void _interrupt _far MYIRQ(void)
{
	unsigned int tmp = toDisp[0];
	unsigned char readin;
	toDisp[0] = toDisp[1];
	toDisp[1] = toDisp[2];
	toDisp[2] = toDisp[3];
	toDisp[3] = tmp;
	
	readin = io_read(L8255PC);
	readin ^= 0x80;
	io_write_byte(L8255PC, readin);
}

// Function name: FeedDog
// Feed the watchdog, with period as the timer for watchdog
// Input:
//		period, type word, the timer for watchdog
// Output: None
// 函数名称：FeedDog
// 看门狗喂狗程序
// 输入：
//		period, 字类型，看门狗计时器计数
//	输出：无
void FeedDog(unsigned int period)
{
	io_write_word(L8253T2, period);
	period >>= 8;
	io_write_word(L8253T2, period);
}

