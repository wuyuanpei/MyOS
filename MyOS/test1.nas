[FORMAT "WCOFF"]
[INSTRSET "i486p"]
[BITS 32]
[FILE "test1.nas"]

	GLOBAL	_HariMain
	EXTERN  _print_str

[SECTION .text]

_HariMain:
	PUSH	msg
	call	_print_str
	POP	EAX
	RET

[SECTION .data]

msg:
	DB	"WorldGG"
	DB	0