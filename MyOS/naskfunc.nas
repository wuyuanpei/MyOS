; naskfunc
; TAB=4

[FORMAT "WCOFF"]	
[INSTRSET "i486p"]				; i486 architecture
[BITS 32]						; 32 bit program
[FILE "naskfunc.nas"]			; File name
		; Global Symbols
		GLOBAL	_io_hlt, _io_cli, _io_sti, _io_stihlt
		GLOBAL	_io_in8,  _io_in16,  _io_in32
		GLOBAL	_io_out8, _io_out16, _io_out32
		GLOBAL	_io_load_eflags, _io_store_eflags
		GLOBAL	_load_gdtr, _load_idtr, _load_tr
		GLOBAL	_load_cr0, _store_cr0
		GLOBAL	_asm_inthandler20, _asm_inthandler21, _asm_inthandler2c
		GLOBAL	_asm_inthandler0d,_asm_inthandler0c,_asm_inthandler00,_asm_inthandler06
		GLOBAL  _memtest_sub
		GLOBAL  _farjmp, _start_app, _end_app
		GLOBAL  _api_call ;API
		EXTERN	_inthandler20, _inthandler21, _inthandler2c
		EXTERN	_inthandler0d,_inthandler0c,_inthandler00,_inthandler06
		EXTERN  _api_selection
		
[SECTION .text]

_io_hlt:	; void io_hlt(void);
		HLT
		RET

_io_cli:	; void io_cli(void);
		CLI
		RET

_io_sti:	; void io_sti(void);
		STI
		RET

_io_stihlt:	; void io_stihlt(void);
		STI
		HLT
		RET

_io_in8:	; int io_in8(int port);
		MOV		EDX,[ESP+4]		; port
		MOV		EAX,0
		IN		AL,DX
		RET

_io_in16:	; int io_in16(int port);
		MOV		EDX,[ESP+4]		; port
		MOV		EAX,0
		IN		AX,DX
		RET

_io_in32:	; int io_in32(int port);
		MOV		EDX,[ESP+4]		; port
		IN		EAX,DX
		RET

_io_out8:	; void io_out8(int port, int data);
		MOV		EDX,[ESP+4]		; port
		MOV		AL,[ESP+8]		; data
		OUT		DX,AL
		RET

_io_out16:	; void io_out16(int port, int data);
		MOV		EDX,[ESP+4]		; port
		MOV		EAX,[ESP+8]		; data
		OUT		DX,AX
		RET

_io_out32:	; void io_out32(int port, int data);
		MOV		EDX,[ESP+4]		; port
		MOV		EAX,[ESP+8]		; data
		OUT		DX,EAX
		RET

_io_load_eflags:	; int io_load_eflags(void);
		PUSHFD		; PUSH EFLAGS 
		POP		EAX
		RET

_io_store_eflags:	; void io_store_eflags(int eflags);
		MOV		EAX,[ESP+4]
		PUSH	EAX
		POPFD		; POP EFLAGS 
		RET
		
_load_gdtr:		; void load_gdtr(int limit, int addr);
		MOV		AX,[ESP+4]		; limit
		MOV		[ESP+6],AX
		LGDT	[ESP+6]
		RET

_load_idtr:		; void load_idtr(int limit, int addr);
		MOV		AX,[ESP+4]		; limit
		MOV		[ESP+6],AX
		LIDT	[ESP+6]
		RET

_load_tr:		; void load_tr(int tr);
		LTR		[ESP+4]			; tr
		RET

_asm_inthandler20:
		PUSH	ES
		PUSH	DS
		PUSHAD
		MOV		EAX,ESP
		PUSH	EAX
		MOV		AX,SS
		MOV		DS,AX
		MOV		ES,AX
		CALL	_inthandler20
		POP		EAX
		POPAD
		POP		DS
		POP		ES
		IRETD

_asm_inthandler21:
		PUSH	ES
		PUSH	DS
		PUSHAD
		MOV		EAX,ESP
		PUSH	EAX
		MOV		AX,SS
		MOV		DS,AX
		MOV		ES,AX
		CALL	_inthandler21
		POP		EAX
		POPAD
		POP		DS
		POP		ES
		IRETD

_asm_inthandler2c:
		PUSH	ES
		PUSH	DS
		PUSHAD
		MOV		EAX,ESP
		PUSH	EAX
		MOV		AX,SS
		MOV		DS,AX
		MOV		ES,AX
		CALL	_inthandler2c
		POP		EAX
		POPAD
		POP		DS
		POP		ES
		IRETD

_asm_inthandler00:	; divide by zero exception handler
		STI
		PUSH	ES
		PUSH	DS
		PUSHAD
		MOV		EAX,ESP
		PUSH	EAX
		MOV		AX,SS
		MOV		DS,AX
		MOV		ES,AX
		CALL	_inthandler00
		CMP		EAX,0		; if non-zero, kill the application
		JNE		_end_app
		POP		EAX
		POPAD
		POP		DS
		POP		ES
		ADD		ESP,4		; special for INT 0x0
		IRETD

_asm_inthandler06:	; illegal instruction exception handler
		STI
		PUSH	ES
		PUSH	DS
		PUSHAD
		MOV		EAX,ESP
		PUSH	EAX
		MOV		AX,SS
		MOV		DS,AX
		MOV		ES,AX
		CALL	_inthandler06
		CMP		EAX,0		; if non-zero, kill the application
		JNE		_end_app
		POP		EAX
		POPAD
		POP		DS
		POP		ES
		ADD		ESP,4		; special for INT 0x06
		IRETD

_asm_inthandler0d:	; protected exception handler
		STI
		PUSH	ES
		PUSH	DS
		PUSHAD
		MOV		EAX,ESP
		PUSH	EAX
		MOV		AX,SS
		MOV		DS,AX
		MOV		ES,AX
		CALL	_inthandler0d
		CMP		EAX,0		; if non-zero, kill the application
		JNE		_end_app
		POP		EAX
		POPAD
		POP		DS
		POP		ES
		ADD		ESP,4		; special for INT 0x0d
		IRETD

_asm_inthandler0c:	; stack exception handler
		STI
		PUSH	ES
		PUSH	DS
		PUSHAD
		MOV		EAX,ESP
		PUSH	EAX
		MOV		AX,SS
		MOV		DS,AX
		MOV		ES,AX
		CALL	_inthandler0c
		CMP		EAX,0		; if non-zero, kill the application
		JNE		_end_app
		POP		EAX
		POPAD
		POP		DS
		POP		ES
		ADD		ESP,4		; special for INT 0x0c
		IRETD
		
_load_cr0:		; int load_cr0(void);
		MOV		EAX,CR0
		RET

_store_cr0:		; void store_cr0(int cr0);
		MOV		EAX,[ESP+4]
		MOV		CR0,EAX
		RET
		
;/* Go through the memory to test its size <C_Version> */
;unsigned int memtest_sub(unsigned int start, unsigned int end)
;{
	;/* a: 1010 5: 0101 */
	;unsigned int i, *p, old, pat0 = 0xaa55aa55, pat1 = 0x55aa55aa;
	;/* Go through the memory in a MEMORY_STEP_SIZE block */
	;for (i = start; i <= end; i += MEMORY_STEP_SIZE) {
		;p = (unsigned int *) (i + MEMORY_STEP_SIZE - 4); // The end of the block
		;old = *p;			/* Store the current value */
		;*p = pat0;			/* Write pat0 */
		;*p ^= 0xffffffff;	/* ~pat0 */
		;if (*p != pat1) {	/* Test pat0: not valid */
;not_memory:
			;*p = old;		/* Restore old */
			;break;
		;}
		;*p ^= 0xffffffff;	/* ~pat0 */
		;if (*p != pat0) {	/* Test pat0: not valid */
			;goto not_memory;
		;}
		;*p = old;			/* Restore pat0 */
	;}
	;return i;
;}

; <ASM_Version>
; The step size for going through the memory

MEMORY_STEP_SIZE	EQU		0x400 ; 1 KB

_memtest_sub:	;unsigned int memtest_sub(unsigned int start, unsigned int end);
		PUSH	EDI						; We need EBX, ESI, EDI
		PUSH	ESI
		PUSH	EBX
		MOV		ESI,0xaa55aa55			; pat0 = 0xaa55aa55;
		MOV		EDI,0x55aa55aa			; pat1 = 0x55aa55aa;
		MOV		EAX,[ESP+12+4]			; i = start;
mts_loop:
		MOV		EBX,EAX
		ADD		EBX,MEMORY_STEP_SIZE-4	; p = i + MEMORY_STEP_SIZE - 4;
		MOV		EDX,[EBX]				; old = *p;
		MOV		[EBX],ESI				; *p = pat0;
		XOR		DWORD [EBX],0xffffffff	; *p ^= 0xffffffff;
		CMP		EDI,[EBX]				; if (*p != pat1) goto fin;
		JNE		mts_fin
		XOR		DWORD [EBX],0xffffffff	; *p ^= 0xffffffff;
		CMP		ESI,[EBX]				; if (*p != pat0) goto fin;
		JNE		mts_fin
		MOV		[EBX],EDX				; *p = old;
		ADD		EAX,MEMORY_STEP_SIZE	; i += MEMORY_STEP_SIZE;
		CMP		EAX,[ESP+12+8]			; if (i <= end) goto mts_loop;
		JBE		mts_loop
		POP		EBX
		POP		ESI
		POP		EDI
		RET
mts_fin:
		MOV		[EBX],EDX				; *p = old;
		POP		EBX
		POP		ESI
		POP		EDI
		RET

_farjmp:	; void farjmp(int eip, int cs);
		JMP	FAR	[ESP+4]	; JMP cs:eip
		RET

_api_call:	; API funtion as INT 0x30
		; CLI will be called automatically during INT
		STI
		PUSH	DS
		PUSH	ES
		PUSHAD		; store values in application's stack
		PUSHAD		; arguments to _api_selection
		MOV		AX,SS
		MOV		DS,AX		; set to OS's segment
		MOV		ES,AX

		CALL		_api_selection

		CMP		EAX,0		; When EAX is not 0, application ends
		JNE		_end_app
		ADD		ESP,32
		POPAD
		POP		ES
		POP		DS
		IRETD
_end_app:
;	EAX is tss.esp0 address
		MOV		ESP,[EAX]
		POPAD
		RET		; return to run_program(char *ext)

_start_app:	; void start_app(int eip, int cs, int esp, int ds, int *tss_esp0);
		PUSHAD		; store registers
		MOV		EAX,[ESP+36]	; EIP
		MOV		ECX,[ESP+40]	; CS
		MOV		EDX,[ESP+44]	; ESP
		MOV		EBX,[ESP+48]	; DS/SS
		MOV		EBP,[ESP+52]	; tss.esp0 address
		MOV		[EBP],ESP
		MOV		[EBP+4],SS	; store OS' esp and ss in TSS
		MOV		ES,BX
		MOV		DS,BX
		MOV		FS,BX
		MOV		GS,BX
; Adjust stack to use RETF to call the application
		OR		ECX,3		; CS or 3
		OR		EBX,3		; DS or 3 : tricks to use RETF
		PUSH		EBX
		PUSH		EDX
		PUSH		ECX		; CS
		PUSH		EAX		; EIP
		RETF				; When the segment is set to "application"
						; CPU will automatically manage the segment registers
		
