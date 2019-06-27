#include "syslib.h"

int a = 1233445;
char buf[] = "wodetian";
void HariMain(void) {
	print_str("HelloWorld");
	print_int(a);
	print_str(buf);
	SHEET s = new_window(300,40,50,10,"First App Window");
	draw_str(s, COL_TRANSPARENT, "My String", 20, 10);
	draw_rec(s, COL_TRANSPARENT, 5, 10, 50, 20);
}
