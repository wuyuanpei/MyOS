; MyOS boot asm
; TAB=4

BOTPAK	EQU		0x00280000		; Bootpack Address
DSKCAC	EQU		0x00100000		; Disk in the memory address
DSKCAC0	EQU		0x00008000		; Disk in the memory address (real mode)

; BOOT_INFO
CYLS	EQU		0x0ff0			; Number of sectors loaded
LEDS	EQU		0x0ff1			; Keyboard status
VMODE	EQU		0x0ff2			; Info for color mode
SCRNX	EQU		0x0ff4			; x solution
SCRNY	EQU		0x0ff6			; y solution
VRAM	EQU		0x0ff8			; Starting address of Display Buffer

		ORG		0xc200			; Starting address of the boot program

; Set up GUI

		MOV		AL,0x13			; VGA, 320x200x8bit
		MOV		AH,0x00
		INT		0x10
		MOV		BYTE [VMODE],8	; Screen mode
		MOV		WORD [SCRNX],320
		MOV		WORD [SCRNY],200
		MOV		DWORD [VRAM],0xa0000

; Use BIOS to get keyboard status

		MOV		AH,0x02
		INT		0x16 			; keyboard BIOS
		MOV		[LEDS],AL

; Block PIC interrupt signals

		MOV		AL,0xff
		OUT		0x21,AL			; io_out(PIC0_IMR, 0xff);
		NOP
		OUT		0xa1,AL			; io_out(PIC1_IMR, 0xff);

		CLI						; set IF in CPU

; Set A20GATE

		CALL	waitkbdout		; wait_KBC_sendready();
		MOV		AL,0xd1
		OUT		0x64,AL
		CALL	waitkbdout
		MOV		AL,0xdf			; enable A20 to support 32 bit memory
		OUT		0x60,AL
		CALL	waitkbdout

; real mode to protected mode

[INSTRSET "i486p"]				; using 486 instruction set

		LGDT	[GDTR0]			; set temporary GDT
		MOV		EAX,CR0			; reset CR0 (Control Register 0)
		AND		EAX,0x7fffffff	; clear bit 31 (no paging)
		OR		EAX,0x00000001	; set bit  0 (to protected mode)
		MOV		CR0,EAX
		JMP		pipelineflush	; JMP after CR0 reset
pipelineflush:
		MOV		AX,8			; reset all segment registers (except cs)
		MOV		DS,AX			; set all of them to 0x8 (i.e. gdt+1)
		MOV		ES,AX
		MOV		FS,AX
		MOV		GS,AX
		MOV		SS,AX

; store bootpack

		MOV		ESI,bootpack	; origin
		MOV		EDI,BOTPAK		; destination
		MOV		ECX,512*1024/4	; 512KB
		CALL	memcpy

; store disk content

; boot area (IPL)

		MOV		ESI,0x7c00		; origin
		MOV		EDI,DSKCAC		; destination
		MOV		ECX,512/4		; 512B
		CALL	memcpy

; other stuff in the disk

		MOV		ESI,DSKCAC0+512	; origin
		MOV		EDI,DSKCAC+512	; destination
		MOV		ECX,0
		MOV		CL,BYTE [CYLS]
		IMUL	ECX,512*18*2/4	; CYLS*SECTOR_SIZE*SECTOR*HEAD/4
		SUB		ECX,512/4		; Minus IPL
		CALL	memcpy

; asmhead finished here, ready to start bootpack

; start bootpack

		MOV		EBX,BOTPAK
		MOV		ECX,[EBX+16]
		ADD		ECX,3			; ECX += 3;
		SHR		ECX,2			; ECX /= 4;
		JZ		skip			; No need to copy data, go to the start of bootpack
		MOV		ESI,[EBX+20]
		ADD		ESI,EBX			; origin
		MOV		EDI,[EBX+12]	; destination
		CALL	memcpy
skip:
		MOV		ESP,[EBX+12]	; initial stack address
		JMP		DWORD 2*8:0x0000001b	; start bootpack (second segment, 0x1b offset)

waitkbdout:						; wait_KBC_sendready();
		IN		 AL,0x64
		AND		 AL,0x02
		IN		 AL,0x60		; empty rubbish data in keyboard
		JNZ		waitkbdout
		RET

memcpy:							; store [ESI] to [EDI] (4 bit wise)
		MOV		EAX,[ESI]
		ADD		ESI,4
		MOV		[EDI],EAX
		ADD		EDI,4
		SUB		ECX,1
		JNZ		memcpy
		RET
		
		ALIGNB	16
GDT0:
		RESB	8				; NULL Selector (empty region for gdt0)
		; set_segmdesc(gdt + 1, 0xffffffff, 0x00000000, AR_DATA32_RW); // All memory
		DW		0xffff,0x0000,0x9200,0x00cf	
		; set_segmdesc(gdt + 2, LIMIT_BOTPAK, ADR_BOTPAK, AR_CODE32_ER); // For bootpack
		DW		0xffff,0x0000,0x9a28,0x0047	

		DW		0
GDTR0:					; Note GDTR has 48 bits
		DW		8*3-1	; three entries in temporary GDT
		DD		GDT0	; addr of temporary GDT

		ALIGNB	16
bootpack:
