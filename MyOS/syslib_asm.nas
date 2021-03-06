[FORMAT "WCOFF"]
[INSTRSET "i486p"]
[BITS 32]
[FILE "syslib_asm.nas"]
		;GLOBAL	_end
		GLOBAL	_print_int, _print_str, _print_err, _scan_str
		GLOBAL  _new_window, _draw_str, _draw_rec, _draw_pt, _draw_line, _close_window
		GLOBAL  _space

[SECTION .text]

;_end:		; void end(void);
;		MOV		EAX,0
;		INT		0x30

_print_int:	; void print_int(int);
		PUSH		EAX
		PUSH		ECX
		MOV		EAX,1
		MOV		ECX,[ESP+12]
		INT		0x30
		POP		ECX
		POP		EAX
		RET

_print_str:	; void print_str(char *);
		PUSH		EAX
		PUSH		ECX
		MOV		EAX,2
		MOV		ECX,[ESP+12]
		INT		0x30
		POP		ECX
		POP		EAX
		RET

_new_window:	;SHEET new_window(int width, int height, int x, int y, char *title);
		PUSH		EBX
		PUSH		ECX
		PUSH		EDX
		PUSH		ESI
		PUSH		EDI
		MOV		EAX,3
		MOV		EBX,[ESP+24]
		MOV		ECX,[ESP+28]
		MOV		EDI,[ESP+32]
		MOV		ESI,[ESP+36]
		MOV		EDX,[ESP+40]
		INT		0x30
		POP		EDI
		POP		ESI
		POP		EDX
		POP		ECX
		POP		EBX
		RET

_draw_str:	;void draw_str(SHEET sheet, COLOR color, char *str, int x, int y);
		PUSH		EAX
		PUSH		EBX
		PUSH		ECX
		PUSH		EDX
		PUSH		ESI
		PUSH		EDI
		MOV		EAX,4
		MOV		EBX,[ESP+28]
		MOV		ECX,[ESP+32]
		MOV		EDX,[ESP+36]
		MOV		EDI,[ESP+40]
		MOV		ESI,[ESP+44]
		INT		0x30
		POP		EDI
		POP		ESI
		POP		EDX
		POP		ECX
		POP		EBX
		POP		EAX
		RET

_draw_rec:	;void draw_rec(SHEET sheet, COLOR color, int xi, int yi, int xf, int yf);
		PUSH		EAX
		PUSH		EBX
		PUSH		ECX
		PUSH		EDX
		PUSH		ESI
		PUSH		EDI
		PUSH		EBP
		MOV		EAX,5
		MOV		EBX,[ESP+32]
		MOV		EBP,[ESP+36]
		MOV		ECX,[ESP+40]
		MOV		EDX,[ESP+44]
		MOV		EDI,[ESP+48]
		MOV		ESI,[ESP+52]
		INT		0x30
		POP		EBP
		POP		EDI
		POP		ESI
		POP		EDX
		POP		ECX
		POP		EBX
		POP		EAX
		RET

_space:		;void *space(void);
		MOV		EAX,6
		INT		0x30
		RET

_draw_pt:	;void draw_pt(SHEET sheet, COLOR color, int x, int y);
		PUSH		EAX
		PUSH		EBX
		PUSH		ECX
		PUSH		ESI
		PUSH		EDI
		MOV		EAX,7
		MOV		EBX,[ESP+24]
		MOV		ECX,[ESP+28]
		MOV		EDI,[ESP+32]
		MOV		ESI,[ESP+36]
		INT		0x30
		POP		EDI
		POP		ESI
		POP		ECX
		POP		EBX
		POP		EAX
		RET

_draw_line:	;void draw_line(SHEET sheet, COLOR color, int xi, int yi, int xf, int yf);
		PUSH		EAX
		PUSH		EBX
		PUSH		ECX
		PUSH		EDX
		PUSH		ESI
		PUSH		EDI
		PUSH		EBP
		MOV		EAX,8
		MOV		EBX,[ESP+32]
		MOV		EBP,[ESP+36]
		MOV		ECX,[ESP+40]
		MOV		EDX,[ESP+44]
		MOV		EDI,[ESP+48]
		MOV		ESI,[ESP+52]
		INT		0x30
		POP		EBP
		POP		EDI
		POP		ESI
		POP		EDX
		POP		ECX
		POP		EBX
		POP		EAX
		RET

_print_err:	; void print_err(char *);
		PUSH		EAX
		PUSH		ECX
		MOV		EAX,9
		MOV		ECX,[ESP+12]
		INT		0x30
		POP		ECX
		POP		EAX
		RET

_close_window:	; void close_window(SHEET sheet);
		PUSH		EAX
		PUSH		EBX
		MOV		EAX,10
		MOV		EBX,[ESP+12]
		INT		0x30
		POP		EBX
		POP		EAX
		RET

_scan_str:	; void scan_str(char *buf, int length);
		PUSH		EAX
		PUSH		EBX
		PUSH		ECX
		MOV		EAX,11
		MOV		EBX,[ESP+16]
		MOV		ECX,[ESP+20]
		INT		0x30
		POP		ECX
		POP		EBX
		POP		EAX
		RET