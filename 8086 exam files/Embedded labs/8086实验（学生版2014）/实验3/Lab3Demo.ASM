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
; 功能： 程序框架					                            				   |
; 编写：《嵌入式系统原理与实验》课程组                   				   |
; 版本：3.7
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
L8253T0			EQU		xxH			; Timer0's port number in I/O space
L8253T1			EQU 	xxH			; Timer1's port number in I/O space
L8253T2			EQU 	xxH			; Timer2's port number in I/O space
L8253CS			EQU 	xxH			; 8253 Control Register's port number in I/O space
;
; 8255芯片端口地址 （Port Address):
L8255PA			EQU		xxH			; Port A's port number in I/O space
L8255PB			EQU 	xxH			; Port B's port number in I/O space
L8255PC			EQU 	xxH			; Port C's port number in I/O space
L8255CS			EQU 	xxH			; 8255 Control Register's port number in I/O space

; INTNum Read Back
INTNum_RB		EQU		xxh		; Interrupt Number read back port address.

;
;  中断矢量号定义
IRQNum			EQU		xxh			; 中断矢量号,要根据学号计算得到后更新此定义。

Patch_Proteus	EQU		IN AL, 0	;	Simulation Patch for Proteus, please ignore this line

;=======================================================================
; 宏定义
;=======================================================================
;     此处用宏定义的方式实现了四条汇编指令，是为修补Proteus程序对于外部硬件中断仿真功能的BUG而特殊设计，可以在当应用程序完成初始化工作需要等待外部硬件中断产生时使用，
; 其表现为当外部中断发生时会进入与外部硬件提供的中断号（拨位开关DSW1）对应的中断服务程序，参见程序段中的使用说明。
;
    WaitForHWInt MACRO INTNum_RB	; 
		PUSH	DX		;
		PUSH	AX		;
		MOV 	DX, INTNum_RB		;
		IN		AL, DX	;
		OUT 	0, AL	;
		STI				;
		CLI				;
		POP		AX		;
		POP		DX		;
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
		Patch_Protues				; Simulation Patch for Proteus,
									; Please ignore the above code line.
;------------------------------------------------------------------------

START:								; 代码需要修改，否则无法编译通过
									; 以下的代码可以根据实际需要选用
		CLI							; 关中断（此指令必须位于第一条指令位置执行）
		MOV AX, @DATA				;
		MOV DS, AX					; 初始化DS段寄存器

		CALL INIT8255				; 初始化8255 
		CALL INIT8253				; 初始化8253
		
;		MOV  BL, IRQNum				; 取得中断矢量号
;		CALL INT_INIT				; 初始化中断向量表

Display_Again:
		CALL DISPLAY8255			; 驱动四位七段数码管显示
; Put you code here


;===================================================================================
; Attention:
; The following codes is a Patching for Proteus 8086 Hardware Simulation Bug.
; Use these codes in the case you want the 8086 to halt and waiting for HW INT only! 
; You can treat it as if it doesn't exist. 
;
; If you need use HW INT, please uncomment it, or
; Don't modify it, leave it here just as it is.
;		WaitForHWInt INTNum_RB		; 8086中断仿真补丁代码，直接忽略此代码行的存在即可
;====================================================================================
		JMP	Display_Again

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

; 发送控制字到8255

		RET
INIT8255 ENDP

;--------------------------------------------
;                                           |
; INIT 8253 					            |
;                                           |
;--------------------------------------------
INIT8253 PROC

;	设定Timer0的初始值
; Put your code here

;	设定Timer1的初始值
; Put your code here

;	设定Timer2的初始值
; Put your code here

		RET
INIT8253 ENDP
;--------------------------------------------
;                                           |
; DISPLAY  STUDENTS ID				 		|
;                                           |
;--------------------------------------------

DISPLAY8255 PROC
; Put your code here



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
; Put your code here

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