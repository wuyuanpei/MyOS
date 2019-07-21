#include "syslib.h"

void HariMain(void) {
	int i, x, y;
	SHEET s = new_window(100,100,450,300,"stars");
	draw_rec(s, COL_BLACK, 3, 22, 96, 96);
	for (i = 0; i < 50; i++) {
		x = (rand() % 94) +  3;
		y = (rand() % 75) + 22;
		draw_pt(s, COL_YELLOW, x, y);
	}
	char buf[2];
	scan_str(buf, 2);
	print_str(buf);
}
