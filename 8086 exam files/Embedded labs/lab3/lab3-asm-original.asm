;--------------------------------------------------------------------------
;
;              Build this with the "Source" menu using
;                     "Build All" option
;
;--------------------------------------------------------------------------
;
;                           实验三程序通用框架
;
;--------------------------------------------------------------------------
; 功能： 采用实验三程序通用框架编写，要求3.1部分					                            				   |
; 编写：《嵌入式系统原理与实验》课程组                   				   |
; 版本：3.4
; 修订：B
;--------------------------------------------------------------------------


		DOSSEG
		.MODEL	SMALL		; 设定8086汇编程序使用Small model
		.8086				; 设定采用8086汇编指令集

;-----------------------------------------------------------
;	定义堆栈段                                             |
;-----------------------------------------------------------
	.stack 100h				; 定义256字节容量的堆栈

;-------------------------------------------------------------------------
;	符号定义                                                              |
;-------------------------------------------------------------------------
;
;
; 8253芯片端口地址 （Port Address):
L8253T0			EQU		100h		; Even Timer0's address in I/O space, 108H as well.
									; 101H, 109H for high order byte access.
L8253T1			EQU 	102h		; Timer1's address in I/O space
									; 103H, 10BH for high order byte access.
L8253T2			EQU 	104h		; Timer2'saddress in I/O space
									; 105H, 10DH for high order byte access.
L8253CS			EQU 	106h		; 8253 Control Register's address in I/O space
									; 107H, 10FH for high order byte access.
;
; 8255芯片端口地址 （Port Address):
L8255PA			EQU		121h		; Odd Port A's address in I/O space, 129H as well.
									; 120H, 128H for low order byte access.
L8255PB			EQU 	123h		; Odd Port B's address in I/O space, 12BH as well.
									; 122H, 12AH for low order byte access.
L8255PC			EQU 	125h		; Odd Port C's address in I/O space, 13DH as well.
									; 124H, 12CH for low order byte access.
L8255CS			EQU 	127h		; 8255 Control Register's port number in I/O space, 12FH as well
									; 126H, 12EH for low order byte access.
;
;  中断矢量号定义
IRQNum			EQU		20h			; 中断矢量号,要根据学号计算得到后更新此定义。

Patch_Proteus	EQU		IN AL, 0	;	Simulation Patch for Proteus, please ignore this line

;=======================================================================
; 宏定义
;=======================================================================

; 修补Proteus仿真的BUG，参见程序段中的使用说明
    WaitForHWInt MACRO INTNum		; INTNum is the HW INT number
	LOCAL LL
		MOV AL, INTNum   			;
	LL:	OUT 0,AL					;
		JMP LL
    ENDM


;-----------------------------------------------------------
;	定义数据段                                             |
;-----------------------------------------------------------
		.data					; 定义数据段;

DelayShort	dw	40				; 短延时参量	
DelayLong	dw	4000			; 长延时参量


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

;-----------------------------------------------------------
;	定义代码段                                             |
;-----------------------------------------------------------
		.code						; Code segment definition
		.startup					; 定义汇编程序执行入口点
;------------------------------------------------------------------------
		Patch_Proteus				; Simulation Patch for Proteus,
									; Please ignore the above code line.
;------------------------------------------------------------------------

START:								; 代码需要修改，否则无法编译通过
									; 以下的代码可以根据实际需要选用

		CALL INIT8255				; 初始化8255 
		CALL INIT8253				; 初始化8253
Display_Again:
		CALL DISPLAY8255			; 驱动四位七段数码管显示
		CALL AccessPC				; Poll PC0, Modify PC6
		JMP  Display_Again			; 
		
;		MOV  BL, IRQNum				; 取得中断矢量号
;		CALL INT_INIT				; 初始化中断向量表
;		STI							; 开中断

;===================================================================================
; Attention:
; The following codes is a Patching for Proteus 8086 Hardware Simulation Bug.
; Use these codes in the case you want the 8086 to halt and waiting for HW INT only! 
; You can treat it as just the following assembly line:
;
;Locallable: 	JMP Locallable	; Endless loop
;
; If you need this function, uncomment it, or
; Please don't modify it, leave it just as it is.
;====================================================================================
;	WaitForHWInt	IRQNum			; 8086中断仿真补丁代码，可根据需要使用。

		HLT							; 停止主程序运行
;=====================================================================================




;--------------------------------------------
;	子程序定义
;
;--------------------------------------------
;                                           |
; INIT 8255 					            |
;                                           |
;--------------------------------------------
INIT8255 PROC

; Init 8255 in Mode x,	L8255PA xPUT, L8255PB xPUT,	L8255PCU xPUT, L8255PCL xPUT
;	计算8255的控制字
; Init 8255 in Mode 0,	L8255PA Output, L8255PB Output,	L8255PC UP Input, L8255PC LOW Output
		MOV DX, L8255CS
		MOV AL, 10000001B		; Control Word
; 发送控制字到8255
		OUT DX, AL
		RET
INIT8255 ENDP

;--------------------------------------------
;                                           |
; INIT 8253 					            |
;                                           |
;--------------------------------------------
INIT8253 PROC

;	计算8253的控制字

;	设定Timer0的初始值
;	设定Timer1的初始值
;	设定Timer2的初始值

		RET
INIT8253 ENDP

;-------------------------------------------
;                                           |
; Function: Polling 8255 PC0, send to PC6   |
; Input: None                               |
; Output: None                              |
;-------------------------------------------
AccessPC	PROC
;	读取C口数值，并把PC0的值输出到PC6。
		MOV	DX, L8255PC		; DX指向C端口
		IN  AL, DX			; 读取C口数值
		MOV AH, AL			; Save it
		AND AL, 00000001b	; Test bit 0
		MOV AH, 10111111b	; 清除PC6
		JZ  APCOut
		OR  AH, 01000000b	; 置位PC6
APCOut:
		MOV AL, AH			; Restore and Modify
		OUT DX, AL			; 更新PC6
		RET
AccessPC	ENDP
;--------------------------------------------
;                                           |
; DISPLAY  STUDENTS ID				 		|
;                                           |
;--------------------------------------------

DISPLAY8255 PROC
		
;	点亮第一个七段管
		MOV DX, L8255PA		; 指向PA
		MOV AL, 0FEh		; 设定第一个七段管的PA值
		OUT DX, AL			; 输出到PA
		MOV AL, SEGTAB+9	; 获取字模‘9’的值
		MOV DX, L8255PB		; 指向PB
		OUT DX, AL			; 输出字模到PB口，点亮
		CALL DELAY			; 延时

;	点亮第二个七段管
		MOV DX, L8255PA		; 指向PA
		MOV AL, 0FDh		; 设定第一个七段管的PA值
		OUT DX, AL			; 输出到PA
		MOV AL, SEGTAB+1	; 获取字模‘1’的值
		MOV DX, L8255PB		; 指向PB
		OUT DX, AL			; 输出字模到PB口，点亮
		CALL DELAY			; 延时

;	点亮第三个七段管
		MOV DX, L8255PA		; 指向PA
		MOV AL, 0FBh		; 设定第一个七段管的PA值
		OUT DX, AL			; 输出到PA
		MOV AL, SEGTAB+5	; 获取字模‘5’的值
		MOV DX, L8255PB		; 指向PB
		OUT DX, AL			; 输出字模到PB口，点亮
		CALL DELAY			; 延时

;	点亮第四个七段管
		MOV DX, L8255PA		; 指向PA
		MOV AL, 0F7h		; 设定第一个七段管的PA值
		OUT DX, AL			; 输出到PA
		MOV AL, SEGTAB+7	; 获取字模‘7’的值
		MOV DX, L8255PB		; 指向PB
		OUT DX, AL			; 输出字模到PB口，点亮
		CALL DELAY			; 延时

		RET
	
DISPLAY8255 ENDP


;--------------------------------------------------------------
;                                                             |                                                            |
; Function：DELAY FUNCTION                                    | 
; Input：None												  |
; Output: None                                                |
;--------------------------------------------------------------

DELAY 	PROC
    	PUSH CX
    	MOV CX, DelayShort
D1: 	LOOP D1
    	POP CX
    	RET
DELAY 	ENDP

;-------------------------------------------------------------
;                                                             |                                                            |
; Function：INTERRUPT Vector Table INIT						  |
; Input: BL = Interrupt number								  |
; Output: None			                                	  |
;                                                             |
;-------------------------------------------------------------	
INT_INIT	PROC FAR			; 此部分程序有删节，需要修改
		CLI						; Disable interrupt
		MOV AX, 0
		MOV ES, AX				; 准备操作中断向量表

; 提示：可参考使用SET、OFFSET运算子获取标号的段地址值和段内偏移量
		
INT_INIT	ENDP

		
;--------------------------------------------------------------
;                                                             |                                                            |
; FUNCTION: INTERRUPT SERVICE  Routine （ISR）				  | 
; Input::                                                     |
; Output:                                                     |
;                                                             |
;--------------------------------------------------------------	
		
MYIRQ 	PROC FAR				; 此部分程序有删节，需要修改
; Put your code here

								; 中断返回
MYIRQ 	ENDP

	END						; 指示汇编程序结束编译