[INSTRSET "i486p"]
[BITS 32]
	MOV		ECX,10086
	MOV		EAX,1
	INT		0x30
	MOV		ECX,info
	MOV		EAX,2
	INT		0x30
	RETF
info:	
	DB		"HelloWorld"
	DB		0
	