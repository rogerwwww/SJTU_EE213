;-----------------------------------------------------------
;实验一 第四题                                             |
;                                                          |
;功能：初始化0x0000到0x8FFF内存                            |
;内容为00h到FFh循环                                        |
;                                                          |
;作者：汪润中 515021910285                                 |
;-----------------------------------------------------------


		.MODEL	SMALL			; 设定8086汇编程序使用Small model
		.8086					; 设定采用8086汇编指令集

;-----------------------------------------------------------
;	符号定义                                               |
;-----------------------------------------------------------


;-----------------------------------------------------------
;	定义堆栈段                                             |
;-----------------------------------------------------------
		.stack 100h				; 定义256字节容量的堆栈

;-----------------------------------------------------------
;	定义代码段                                             |
;-----------------------------------------------------------
		.code						; Code segment definition
		.startup					; 定义汇编程序执行入口点
;
;以下开始放置用户指令代码
;
;-----------------------------------------------------------
;主程序部分,将0x0000到0x8FFF按要求初始化，                 |
;0x9000开始为指令和堆栈			                           |
;-----------------------------------------------------------

MAIN	PROC
		MOV AX,0000H				;初始化AX
setAllWords:						;按要求初始化全部内存
		MOV DS,AX					;将DS移到开始位置
		CALL setAWord				;第一次循环设置0x0000至0x00FF的值，以此类推
		ADD AX,10H					;更新AX，后续赋值给DS
		CMP AX,0900H				;比较是否超出上界
		JBE setAllWords				;没有超出上界，则继续执行
		HLT							;程序执行完毕，挂起
		MAIN ENDP

setAWord	PROC
		PUSH CX						;堆栈中暂存数据
		PUSH BX
		PUSH AX
		MOV CX,80H					;初始化循环变量
wordloop:
		MOV BX,CX
		DEC BX
		SHL BX,1
		MOV AL,BL
		MOV AH,BL
		INC AH
		MOV [BX],AX					;BX存储目标地址，AX存储将要赋给改地址的值
		LOOP wordloop				;循环
		POP AX						;从堆栈中取出数据
		POP BX
		POP CX
		RET		
		setAWord ENDP
		
		END					;指示汇编程序结束编译
		