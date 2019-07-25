#include "syslib.h"

void HariMain(void) {
	int i, x, y;
	SHEET s = new_window(100,100,100,100,"stars");
	draw_rec(s, COL_BLACK, 3, 22, 96, 96);
	for (i = 0; i < 50; i++) {
		x = (rand() % 94) +  3;
		y = (rand() % 75) + 22;
		draw_pt(s, COL_YELLOW, x, y);
	}
	draw_str(s, COL_WHITE, "Hello", 20, 20);
	draw_line(s, COL_RED, 16,29,85,90);
	char buf[2];
	scan_str(buf, 2);
	print_str(buf);
}
