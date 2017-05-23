;-----------------------------------------------------------
;实验一 第四题                                             |
;                                                          |
;功能：读取开关量的状态取反后送到LED显示                   |
;74LS244带有8位拨位开关，地址为90H or 92H or 94H or 96H    |
;74LS273带有8个发光二极管，地址为A0H or A2H or A4H or A6H  |
;                                                          |
;作者：汪润中 515021910285                                 |
;-----------------------------------------------------------


		.MODEL	SMALL			; 设定8086汇编程序使用Small model
		.8086					; 设定采用8086汇编指令集

;-----------------------------------------------------------
;	符号定义                                               |
;-----------------------------------------------------------
;
PortIn	EQU	90H		;定义输入端口号
PortOut	EQU	0A0H	;定义输出端口号
;-----------------------------------------------------------
;	定义堆栈段                                             |
;-----------------------------------------------------------
		.stack 100h				; 定义256字节容量的堆栈

;-----------------------------------------------------------
;	定义数据段                                             |
;-----------------------------------------------------------
		.data					; 定义数据段
;MyVar	DB	?					; 定义MyVar变量   


;-----------------------------------------------------------
;	定义代码段                                             |
;-----------------------------------------------------------
		.code						; Code segment definition
;		.startup					; 定义汇编程序执行入口点
;
;以下开始放置用户指令代码
;
;-----------------------------------------------------------
;主程序部分,读取开关量状态取反后送显示                     |
;-----------------------------------------------------------

Again:
		MOV AL,0		;
		IN  AL,PortIn		;读取开关量状态
		NOT AL				;取反
		OUT PortOut,AL		;送显示
		JMP Again			;跳转循环执行

		END					;指示汇编程序结束编译
		