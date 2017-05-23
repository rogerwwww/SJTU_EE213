;-----------------------------------------------------------
;实验一示例程序                                            |
;                                                          |
;功能：读取开关量的状态取反后送到LED显示                   |
;74LS244带有8位拨位开关，地址为80H or 82H or 84H or 86H    |
;74LS273带有8个发光二极管，地址为88H or 8AH or 8CH or 8EH  |
;                                                          |
;Designer：SYZ                                             |
;-----------------------------------------------------------


		.MODEL	SMALL			; 设定8086汇编程序使用Small model
		.8086				; 设定采用8086汇编指令集

;-----------------------------------------------------------
;	符号定义                                               |
;-----------------------------------------------------------
;
PortOut	EQU	88h	;定义输出端口号


;-----------------------------------------------------------
;	定义堆栈段                                             |
;-----------------------------------------------------------
		.stack 100h				; 定义256字节容量的堆栈

;-----------------------------------------------------------
;	定义数据段                                             |
;-----------------------------------------------------------
		.data					; 定义数据段
MyVar	DB	93H					; 定义MyVar变量,Binary 10010011, both red on
FCOUNTER	DB	?
DELAY	DW	40000
DELAY_Y	DW	4000
FLASH	DW	1000


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
MAIN	PROC
Again:
;STAGE 1
		MOV AL,00110011B			;Green on & Red on
		OUT PortOut,AL				;输出显示
		CALL LongDelay				;delay of STAGE 1
;STAGE 1.5
		MOV CX,03H					;set flash counter
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
		
		JMP Again
		
MAIN ENDP

		
LongDelay	PROC
		PUSH CX
		MOV CX,DS:DELAY
longloop:	LOOP longloop
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