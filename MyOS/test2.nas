[FORMAT "WCOFF"]
[INSTRSET "i486p"]
[BITS 32]
[FILE "test2.nas"]

	GLOBAL	_HariMain

[SECTION .text]

_HariMain:
	DB	'Hello'
	RET
