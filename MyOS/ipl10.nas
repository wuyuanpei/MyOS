; MyOS-ipl
; TAB=4

CYLS	EQU		10				; Load CYLS cylinders into memory
ERRN	EQU		50				; Maximum number of times to load while
							; a section causes error
		ORG		0x7c00			; The standard loading address for ipl

; Standard FAT12 format floppy code

		JMP		entry
		DB		0x90
		DB		"BOOTSECT"		; Name for loading sector
		DW		512				; Sector size
		DB		1				; Cluster size (in number of sectors)
		DW		1				; Starting position of FAT
		DB		2				; Number of FAT
		DW		224				; Size of root directory 
								; (in number of items)
		DW		2880			; Disk size (1440*1024/512)
		DB		0xf0			; Disk type (0xf0)
		DW		9				; FAT length (9 sectors)
		DW		18				; Number of sectors in one track
		DW		2				; Number of heads
		DD		0				; No disk partition (0)
		DD		2880			; Disk size (1440*1024/512)
		DB		0,0,0x29
		DD		0xffffffff
		DB		"MYOS_DISK  "	; Disk name
		DB		"FAT12   "		; Disk format name
		RESB	18

; Main Program

entry:

; Initialize SS SP and SD

		MOV		AX,0			
		MOV		SS,AX
		MOV		SP,0x7c00
		MOV		DS,AX

; Read Disk

		MOV		AX,0x0820
		MOV		ES,AX			; ES:BX writing address
		MOV		CH,0			; Cylinder 0
		MOV		DH,0			; Head 0
		MOV		CL,2			; Sector 2
readloop:
		MOV		SI,0			; Inialize error count
retry:
		MOV		AH,0x02			; Read from disk (AH=0x02)
		MOV		AL,1			; Read one sector
		MOV		BX,0
		MOV		DL,0x00			; A disk driver
		INT		0x13			; Call BIOS
		JNC		next			; Jump if no error
		ADD		SI,1			; Increment error count
		CMP		SI,ERRN			; Compare error with 
		JAE		error			; SI>=ERRN Jump to error
		MOV		AH,0x00
		MOV		DL,0x00
		INT		0x13			; Reset Disk Driver
		JMP		retry
next:
		MOV		AX,ES
		ADD		AX,0x0020
		MOV		ES,AX			; ADD ES to 0x020 (Next sector)
		ADD		CL,1
		CMP		CL,18
		JBE		readloop		; Continue reading sectors in the track
		MOV		CL,1
		ADD		DH,1			; Go to the back of the disk
		CMP		DH,2
		JB		readloop
		MOV		DH,0
		ADD		CH,1			; Go to next track
		CMP		CH,CYLS
		JB		readloop		; CH<CYLS Continue reading this track

; Loading finished! Save the number of cylinders read.

		MOV		[0x0ff0],CH
		JMP		0xc200			; Go to the beginning of .sys (asmhead.nas)

error:
		MOV		SI,msg			; Display error message

displayloop:
		MOV		AL,[SI]
		ADD		SI,1			; Next character
		CMP		AL,0
		JE		sleep
		MOV		AH,0x0e			; Display one charater
		MOV		BX,15			; Set color
		INT		0x10			; Call BIOS
		JMP		displayloop

sleep:
		HLT						; Halt
		JMP		sleep			; Infinite loop

msg:
		DB		0x0a, 0x0a
		DB		"ERROR: load error"
		DB		0x0a
		DB		0

		RESB	0x7dfe-$		; Fill zeros

		DB		0x55, 0xaa		; Set booting sector footer tag
