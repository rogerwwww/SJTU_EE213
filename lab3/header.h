#ifndef __header_h
#define __header_h
//==============================================================================
// I/O port address defination
// I/O端口地址声明
//------------------------------------------------------------------------------
// Port address of 8253
// 8253芯片端口地址
#define L8253T0 100h		//Even Timer0's address in I/O space, 108H as well.
							//101H, 109H for high order byte access.
#define L8253T1	102h		// Timer1's address in I/O space
							// 103H, 10BH for high order byte access.
#define L8253T2 104h		// Timer2'saddress in I/O space
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

// Interrupt vector defination 
// 中断矢量号定义
#define IRQNum	20h			// Interrupt vector number. It should be calculated from your student number according to the assignment.
							// 中断矢量号,要根据学号计算得到后更新此定义。

// Functions declaration
// 函数声明

//==============================================================================
// I/O functions
// 外围I/O函数
//------------------------------------------------------------------------------
// Function name: io_write_word
// Write a word(outword) to the target address(addr). Written in asm.
// Inputs:
//		addr, type byte, the target address for output data
//		outword, type word(two bytes), the data to be written
// Output: None
// 函数名称： io_write_word
// 向目标地址（addr）写入一个字长度的数据（outword），汇编实现
// 输入：
//		addr，字节类型，输出数据的目标地址
//		outword，字类型，输出数据
// 输出：无
void io_write_word(unsigned int addr, unsigned int outword);

// Funcion name: io_write_byte
// Write a byte(outbyte) to the target address(addr). Written in asm.
// Inputs:
//		addr, type byte, the target address for output data
//		outword, type byte, the data to be written
// Output: None
// 函数名称： io_write_byte
// 向目标地址（addr）写入一个字节长度的数据（outbyte），汇编实现
// 输入：
//		addr，字节类型，输出数据的目标地址
//		outword，字节类型，输出数据
// 输出：无
void io_write_byte(unsigned int addr, unsigned char outbyte);

// Function name: io_read
// Read a word from the target address(addr). Written in asm.
// The returned value could be assigned to a byte type, taking only lower bits.
// Inputs:
//		addr, type byte, the target address to read data from
// Output: 
//		type word, the data read from the target address
// 函数名称： io_read
// 从目标地址（addr）读取一个字长度的数据，汇编实现
// 返回值可以直接赋值给字节类型的变量。若如此做，只有低八位被读取
// 输入：
//		addr，字节类型，读取数据的目标地址
// 输出：
//		字类型，读取到的数据
unsigned int io_read(unsigned int addr);

void DELAY(unsigned int delayCycle);
void INIT8255(void);

// 8253 Mode Notes:
// Mode 0 - Single one-shot, enabled by GATE, for watchdogs
// Mode 1 - Hardware retriggerable one-shot, triggered by GATE
// Mode 2 - Rate generator, enabled by GATE. Output LOW when down to 1, automatically restart
// Mode 3 - Square wave generator, enabled by GATE. Period = N/2, duty cycle 50%(for even Ns)
// Mode 4 - Software triggered strobe, slightly different to Mode 2
// Mode 5 - Hardware triggered strobe
void INIT8253(unsigned int time, unsigned int counter, unsigned int mode);
void AccessPC(void);
void DISPLAY8255(unsigned int segment, unsigned int displayNum);
void INT_INIT(void);
void _interrupt _far MYIRQ(void);
void FeedDog(unsigned int period);

#endif