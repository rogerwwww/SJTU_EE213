#ifndef __header_h
#define __header_h
//==============================================================================
// I/O port address definition
// I/O端口地址声明
//------------------------------------------------------------------------------
// Port address of 8253
// 8253芯片端口地址
#define L8253T0 100h		//Even Timer0's address in I/O space, 108H as well.
							//101H, 109H for high order byte access.
#define L8253T1	102h		// Timer1's address in I/O space
							// 103H, 10BH for high order byte access.
#define L8253T2 104h		// Timer2's address in I/O space
							// 105H, 10DH for high order byte access.
#define L8253CS 106h		// 8253 Control Register's address in I/O space
							// 107H, 10FH for high order byte access.

// Port address of 8255 
// 8255芯片端口地址
#define L8255PA 121h		// Odd Port A's address in I/O space, 129H as well.
							// 120H, 128H for low order byte access.
#define L8255PB 123h		// Odd Port B's address in I/O space, 12BH as well.
							// 122H, 12AH for low order byte access.
#define L8255PC 125h		// Odd Port C's address in I/O space, 13DH as well.
							// 124H, 12CH for low order byte access.
#define L8255CS	127h		// 8255 Control Register's port number in I/O space, 12FH as well
							// 126H, 12EH for low order byte access.

// Interrupt vector definition 
// 中断矢量号定义
#define IRQNum	20h			// Interrupt vector number. It should be calculated from your student number according to the assignment.
							// 中断矢量号,要根据学号计算得到后更新此定义。

// Functions declaration
// 函数声明

//==============================================================================
// I/O functions
// 外围I/O函数
//------------------------------------------------------------------------------
void io_write_word(unsigned int addr, unsigned int outword);

void io_write_byte(unsigned int addr, unsigned char outbyte);

unsigned int io_read(unsigned int addr);

//==============================================================================
// Initialization functions
// 功能初始化函数
//------------------------------------------------------------------------------
void INIT8255(void);

void INIT8253(unsigned int count_time, unsigned int timer, unsigned int mode);

void INT_INIT(void);

//==============================================================================
// Other functions
// 其他功能函数
//------------------------------------------------------------------------------
void AccessPC(void);

void DELAY(unsigned int delayCycle);

void DISPLAY8255(unsigned int segment, unsigned char displayNum);

void _interrupt _far MYIRQ(void);

void FeedDog(unsigned int period);

//==============================================================================
#endif