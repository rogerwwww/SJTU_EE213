;-----------------------------------------------------------
;实验一 第三题                                             |
;                                                          |
;功能：实现路口红绿灯循环控制和显示                        |
;74LS273带有8个发光二极管，地址为88H or 8AH or 8CH or 8EH  |
;                                                          |
;作者：汪润中 515021910285                                 |
;-----------------------------------------------------------

		.MODEL	SMALL			; 设定8086汇编程序使用Small model
		.8086					; 设定采用8086汇编指令集

;-----------------------------------------------------------
;	符号定义                                               |
;-----------------------------------------------------------

PortOut	EQU	88h	;定义输出端口号

;-----------------------------------------------------------
;	定义堆栈段                                             |
;-----------------------------------------------------------
		.stack 100h				; 定义256字节容量的堆栈

;-----------------------------------------------------------
;	定义数据段                                             |
;-----------------------------------------------------------
		.data					; 定义数据段
DELAY	DW	23500

;-----------------------------------------------------------
;	定义代码段                                             |
;-----------------------------------------------------------
		.code						; Code segment definition
;		.startup					; 定义汇编程序执行入口点
;
;以下开始放置用户指令代码
;
;-----------------------------------------------------------
;主程序部分,循环控制路口红绿灯并送显示                     |
;-----------------------------------------------------------
MAIN	PROC
Again:
;STAGE 1
		MOV AL,00110011B			;Green on & Red on
		OUT PortOut,AL				
		CALL LongLongDelay				;delay of STAGE 1
;STAGE 1.5, Green light flash
		MOV CX,04H					;set flash counter
S1p5:
		MOV AL,00110011B			;Green on & Red on
		OUT PortOut,AL
		CALL ShortDelay
		
		MOV AL,00110111B			;Green off & Red on
		OUT PortOut,AL
		CALL ShortDelay
		
		LOOP S1p5					;Loop STAGE 1.5 - green light flash
		
;STAGE 2		
		MOV AL,00110101B			;Yellow on & Red on
		OUT PortOut,AL
		CALL LongDelay
				
;STAGE 3
		MOV AL,00011110B			;Red on & Green on
		OUT PortOut,AL
		CALL LongLongDelay
		
;STAGE 3.5, Green light flash
		MOV CX,04H					;set flash counter
S3p5:
		MOV AL,00011110B			;Red on & Green on
		OUT PortOut,AL
		CALL ShortDelay
		
		MOV AL,00111110B			;Red on & Green off
		OUT PortOut,AL
		CALL ShortDelay
		
		LOOP S3p5
		
;STAGE 4
		MOV AL,00101110B			;Red on & Yellow on
		OUT PortOut,AL
		CALL LongDelay
		
		JMP Again

MAIN ENDP

;-----------------------------------------------------------
;函数部分，三种不同时长的延迟函数                          |
;-----------------------------------------------------------

LongLongDelay	PROC
		PUSH CX
		MOV CX,5
longlongloop:	CALL LongDelay
				Loop longlongloop
		POP CX
		RET
LongLongDelay ENDP

		
LongDelay	PROC
		PUSH CX
		MOV CX,2
longloop:	CALL ShortDelay
			LOOP longloop
		POP CX
		RET
LongDelay	ENDP


ShortDelay	PROC
		PUSH CX
		MOV CX,DS:DELAY
shortloop:	LOOP shortloop
		POP CX
		RET
ShortDelay	ENDP

		END				;指示汇编程序结束编译
		