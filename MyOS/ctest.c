#include "syslib.h"

int a = 1233445;
char buf[] = "wodetian";
char *buf2;
void HariMain(void) {
	int i = 0, j;
	print_str("HelloPPP");
	print_int(a);
	buf2 = (char *)space();
	for( ;i<1024*102 - 1;i++)
		buf2[i] = 'a';
	buf2[1024*102 - 1] = 0;
	//print_str(buf2);
	SHEET s;
	for (j = 0; j < 0x1ff; j ++)
	{
		s = new_window(200,150,425,200,"App");
		close_window(s);
	}
	s = new_window(200,150,425,200,"App");
	draw_str(s, COL_RED, buf2, 20, 10);
	//draw_rec(s, COL_TRANSPARENT, 5, 10, 50, 20);
	draw_line(23, COL_D_BLUE, 5, 9, 1000000003, 120);
	print_str("First non-error");
	print_err("First error");
	int c = 0;
	for(; c < 0xbffffff; c++){
		if(c%0x3ffff == 0)
			draw_line(s, COL_D_BLUE, c%161, c%143,c%191, c%147);
	}
	//close_window(s);
	
}
