;-----------------------------------------------------------
;
;              Build this with the "Source" menu using
;                     "Build All" option
;
;-----------------------------------------------------------
;
;                           实验二示例程序 

;-----------------------------------------------------------
;                                                          |
;                                                          |
; 功能：控制7段数码管的显示                                |
; 编写：《嵌入式系统原理与实验》课程组                     |
;-----------------------------------------------------------
		DOSSEG
		.MODEL	SMALL		; 设定8086汇编程序使用Small model
		.8086				; 设定采用8086汇编指令集
;-----------------------------------------------------------
;	符号定义                                               |
;-----------------------------------------------------------
;
; 8255芯片端口地址 （Port number）分配:
PortA	EQU		91H			; Port A's port number in I/O space
PortB	EQU 	93H			; Port B's port number in I/O space
PortC	EQU 	95H			; Port C's port number in I/O space
CtrlPT	EQU 	97H			; 8255 Control Register's port number in I/O space


;-----------------------------------------------------------
;	定义数据段                                             |
;-----------------------------------------------------------
		.data					; 定义数据段;

DelayShort	dw	4000			; 短延时参量	
DelayLong	dw	40000			; 长延时参量

; 显示数字
DISCHAR DB 01,02,03,04

; SEGTAB是显示字符0-F，其中有部分数据的段码有错误，请自行修正
SEGTAB  DB 3FH	; 7-Segment Tube, 共阴极类型的7段数码管示意图
		DB 06H	;
		DB 5BH	;            a a a
		DB 4FH	;         f         b
		DB 66H	;         f         b
		DB 6DH	;         f         b
		DB 7DH	;            g g g 
		DB 07H	;         e         c
		DB 7FH	;         e         c
		DB 6FH	;         e         c
		DB 77H	;            d d d     h h h
		DB 7CH	; ----------------------------------
		DB 39H	;       b7 b6 b5 b4 b3 b2 b1 b0
		DB 5EH	;       DP  g  f  e  d  c  b  a
		DB 79H	;
		DB 71H	;
		
SWITCH	DB	?	;定义开关变量


;-----------------------------------------------------------
;	定义代码段                                             |
;-----------------------------------------------------------
		.code						; Code segment definition
		.startup					; 定义汇编程序执行入口点
;
; Init 8255 in Mode 0
; PortA Output, PortB Output, PortC Input
;
		MOV AL,10001001B
		OUT CtrlPT,AL		;写入控制寄存器，PA&PB OUTPUT，PC INPUT
;
; 把数字1、2、3、4显示在数码管上
;

L1: 
		IN AL,PortC
		MOV SWITCH,AL				;读入开关数据，存入地址0
		
		MOV AL,SWITCH
		AND AL, 11110000b		;读取开关高4位
		ADD AL,  0Eh			;开关高4位作为高四位，送LED；低四位enable 1号数码管
		OUT PortA,AL			;输出控制数码管和LED显示
		MOV AL,SWITCH
		MOV AH,0
		AND AL, 00001111b		;读取开关低4位
		LEA BX,SEGTAB			;计算SEGTAB的偏移量
		ADD AX,BX				;计算数码管显示量
		MOV BX,AX
		MOV AL,[BX]
		OUT PortB,AL			;输出数码管显示值
		CALL DELAY				;延时以保证数码管显示

		MOV AL,SWITCH
		AND AL, 11110000b		;读取开关高4位
		ADD AL,  0Dh			;开关高4位作为高四位，送LED；低四位enable 1号数码管
		OUT PortA,AL
		MOV AL,SWITCH
		AND AL, 11110000b		;读取开关高4位
		MOV CL,4
		SHR AL,CL				;移位至低4位
		LEA BX,SEGTAB
		ADD AX,BX				;计算数码管显示量
		MOV BX,AX
		MOV AL,[BX]
		OUT PortB,AL			;输出数码管显示值
		CALL DELAY			

		MOV AL,SWITCH
		AND AL, 11110000b
		ADD AL,  0Bh
		OUT PortA,AL
		MOV AL,SWITCH
		MOV AH,0
		AND AL, 00001111b
		LEA BX,SEGTAB
		ADD AX,BX
		MOV BX,AX
		MOV AL,[BX]
		OUT PortB,AL
		CALL DELAY			

		MOV AL,SWITCH
		AND AL, 11110000b
		ADD AL,  7h
		OUT PortA,AL
		MOV AL,SWITCH
		AND AL, 11110000b
		MOV CL,4
		SHR AL,CL
		LEA BX,SEGTAB
		ADD AX,BX
		MOV BX,AX
		MOV AL,[BX]
		OUT PortB,AL
		CALL DELAY		

		JMP L1

RET

;--------------------------------------------
;                                           |
; Delay system running for a while          |
; CX : contains time para.                  |
;                                           |
;--------------------------------------------

DELAY1 	PROC
    	PUSH CX
    	MOV CX,DelayLong	;
D0: 	LOOP D0
    	POP CX
    	RET
DELAY1 	ENDP


;--------------------------------------------
;                                           |
; Delay system running for a while          |
;                                           |
;--------------------------------------------

DELAY 	PROC
    	PUSH CX
    	MOV CX,DelayShort
D1: 	LOOP D1
    	POP CX
    	RET
DELAY 	ENDP


;-----------------------------------------------------------
;	定义堆栈段                                             |
;-----------------------------------------------------------
		.stack 100h				; 定义256字节容量的堆栈


		END						;指示汇编程序结束编译