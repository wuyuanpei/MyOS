#include "syslib.h"

int a = 1233445;
char buf[] = "wodetian";
char *buf2;
void HariMain(void) {
	int i = 0;
	print_str("HelloPPP");
	print_int(a);
	buf2 = (char *)space();
	for( ;i<1024*102 - 1;i++)
		buf2[i] = 'a';
	buf2[1024*102 - 1] = 0;
	//print_str(buf2);
	SHEET s = new_window(200,150,425,200,"App");
	draw_str(s, COL_RED, buf2, 20, 10);
	draw_rec(s, COL_TRANSPARENT, 5, 10, 50, 20);
	draw_line(s, COL_D_BLUE, 5, 9, 1000000003, 120);
	print_str("First non-error");
	print_err("First error");
	
}
