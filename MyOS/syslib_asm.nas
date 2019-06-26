[FORMAT "WCOFF"]
[INSTRSET "i486p"]
[BITS 32]
[FILE "syslib_asm.nas"]
		GLOBAL	_end
		GLOBAL	_print_int
		GLOBAL	_print_str

[SECTION .text]

_end:		; void end(void);
		MOV		EAX,0
		INT		0x30

_print_int:	; void print_int(int);
		MOV		EAX,1
		MOV		ECX,[ESP+4]
		INT		0x30
		RET

_print_str:	; void print_str(char *);
		MOV		EAX,2
		MOV		ECX,[ESP+4]
		INT		0x30
		RET