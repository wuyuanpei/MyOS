#include "bootpack.h"
#include <stdio.h>

/* Information that api needs to know */
extern int malloc_addr;
extern int program_addr;
extern struct SHEET *sheet_created; // If sheet_created is 0, no window created by this application so far

/* API function declarations */
static int *api_terminate(void);
static int *api_print_int(int number);
static int *api_print_str(char *addr);
static int *api_new_window(int *ret, int width, int height, int x, int y, char *title);
static int *api_draw_str(int sheet, unsigned color, char *str, int x, int y);
static int *api_draw_rec(int sheet, unsigned color, int xi, int yi, int xf, int yf);
static int *api_space(int *ret);
static int *api_draw_pt(int sheet, unsigned color, int x, int y);
static int *api_draw_line(int sheet, unsigned color, int xi, int yi, int xf, int yf);
static int *api_print_err(char *addr);
static int *api_close_window(int sheet);
static int *api_scan_str(char *buf, int length);

/* 
 * Select which api to use (called by _api_call) PUSHAD 
 * If return non-zero, the application ends and the return value is tss.esp0
*/
int* api_selection(unsigned int edi, unsigned int esi, unsigned int ebp, unsigned int esp, unsigned int ebx, unsigned int edx, unsigned int ecx, unsigned int eax) {
	int * ret = &eax + 8; // The api return value address (see POPAD in _api_call in naskfunc.nas)
	switch(eax){
		case 0:
			// Terminate application API
			return api_terminate();
		case 1:
			// Print an integer API
			return api_print_int((int)ecx);
		case 2:
			// Print a string API
			return api_print_str((char *)ecx);
		case 3:
			// Create a window API
			return api_new_window(ret, (int)ebx, (int)ecx, (int)edi, (int)esi, (char *)edx);
		case 4:
			// Draw a string on a window API
			return api_draw_str((int)ebx, ecx, (char *)edx, (int)edi, (int)esi);
		case 5:
			// Draw a rectangle on a window API
			return api_draw_rec((int)ebx, ebp, (int)ecx, (int)edx, (int)edi, (int)esi);
		case 6:
			// Return the starting address of .space
			return api_space(ret);
		case 7:
			// Draw a point on a window API
			return api_draw_pt((int)ebx, ecx, (int)edi, (int)esi);
		case 8:
			// Draw a line on a window API
			return api_draw_line((int)ebx, ebp, (int)ecx, (int)edx, (int)edi, (int)esi);
		case 9:
			// Print an error string API
			return api_print_err((char *)ecx);
		case 10:
			// Close the window and free the memory API
			return api_close_window((int)ebx);
		case 11:
			// Scan a string API
			return api_scan_str((char *)ebx, (int)ecx);
	}
	return 0;
}

// Terminate application API
static int *api_terminate(void) {
	return &(task_now()->tss.esp0);
}

// Print an integer API
static int *api_print_int(int number) {
	char buf[15];
	sprintf(buf,"%d",number);
	print_string(buf);
	return 0;
}

// Print a string API
static int *api_print_str(char *addr) {
	print_string(addr + program_addr);
	return 0;
}

// Create a window API
static int *api_new_window(int *ret, int width, int height, int x, int y, char *title) {
	struct SHEET *sht_window;
	unsigned char *buf_window;
	// Every application can only create one window
	if (sheet_created)
	{
		print_error("new_window error: only one window could be created");
		* ret = 0;
		return 0;
	}
	// Initialize sheet and buffer
	io_cli(); // Avoid being terminated when set "sheet_created"
	sht_window = sheet_alloc();
	buf_window = mm_malloc(width * height); // window size
	sheet_setbuf(sht_window, buf_window, width, height, 0xff); // 0xff for col_inv
	make_window(buf_window, width, height, title + program_addr, 0);
	struct SHTCTL *shtctl = (struct SHTCTL *)ADR_SHTCTL;
	sheet_updown(sht_window, shtctl->top);
	sheet_slide(sht_window, x, y); // Position on the screen
	sheet_created = sht_window; // Set the tag
	// Return the sheet handler
	* ret = (int) sht_window;
	io_sti();
	return 0;
}

// Draw a string on a window API
static int *api_draw_str(int sheet, unsigned color, char *str, int x, int y) {
	struct SHEET *sht_window;
	int i;
	sht_window = (struct SHEET *) sheet;
	if (sheet_created == 0 || sheet_created != sht_window)
	{
		print_error("draw_str error: illegal window target");
		return 0;
	}
	// Get the string length
	i = 0; // String length
	while((*((char *)(program_addr + str + i))) && i < 1000){
		i++;
	}
	// Out of window
	if(y > sht_window->bysize - 16 || (x + 8 * i) > sht_window->bxsize){
		print_error("draw_str error: graphics out of window");
		return 0;
	}
	draw_string(sht_window->buf, sht_window->bxsize, color, x, y, program_addr + str);
	sheet_refresh(sht_window, x, y, sht_window->bxsize, y + 16, 1);
	return 0;
}

// Draw a rectangle on a window API
static int *api_draw_rec(int sheet, unsigned color, int xi, int yi, int xf, int yf) {
	struct SHEET *sht_window;
	sht_window = (struct SHEET *) sheet;
	if (sheet_created == 0 || sheet_created != sht_window)
	{
		print_error("draw_rec error: illegal window target");
		return 0;
	}
	// Out of window
	if(yf >= sht_window->bysize || xf >= sht_window->bxsize){
		print_error("draw_rec error: graphics out of window");
		return 0;
	}
	draw_rect(sht_window->buf, sht_window->bxsize, color, xi, yi, xf, yf);
	sheet_refresh(sht_window, xi, yi, xf + 1, yf + 1, 1);
	return 0;
}

// Return the starting address of .space
static int *api_space(int *ret) {
	* ret = (int) malloc_addr;
	return 0;
}

// Draw a point on a window API
static int *api_draw_pt(int sheet, unsigned color, int x, int y) {
	struct SHEET *sht_window;
	sht_window = (struct SHEET *) sheet;
	if (sheet_created == 0 || sheet_created != sht_window)
	{
		print_error("draw_pt error: illegal window target");
		return 0;
	}
	// Out of window
	if(y >= sht_window->bysize || x >= sht_window->bxsize){
		print_error("draw_pt error: graphics out of window");
		return 0;
	}
	sht_window->buf[sht_window->bxsize * y + x] = color;
	sheet_refresh(sht_window, x, y, x + 1, y + 1, 1);
	return 0;
}

// Draw a line on a window API �д��Ľ�
static int *api_draw_line(int sheet, unsigned color, int xi, int yi, int xf, int yf) {
	struct SHEET *sht_window;
	int dx, dy, i, j;
	sht_window = (struct SHEET *) sheet;
	if (sheet_created == 0 || sheet_created != sht_window)
	{
		print_error("draw_line error: illegal window target");
		return 0;
	}
	// xf must be greater (or equal) than xi and yf must be greater (or equal) than yi
	if(xf < xi){ // switch xf and xi
		xf = xf ^ xi;
		xi = xf ^ xi;
		xf = xf ^ xi;
	}
	if(yf < yi){ // switch yf and yi
		yf = yf ^ yi;
		yi = yf ^ yi;
		yf = yf ^ yi;
	}
	// Out of window
	if(yf >= sht_window->bysize || xf >= sht_window->bxsize){
		print_error("draw_line error: graphics out of window");
		return 0;
	}
	// Difference in x and y direction
	dx = xf - xi + 1;
	dy = yf - yi + 1;
	// Two conditions
	if(dx > dy){
		// Go through y
		for(; yi <= yf; yi++){
			i = dx / dy;
			for(j = 0; j < i; j++){
				sht_window->buf[sht_window->bxsize * yi + xi + j] = color;
				sheet_refresh(sht_window, xi + j, yi, xi + j + 1, yi + 1, 1);
			}
			xi += i;
			dx -= i;
			dy --;
		}
	}else{
		// Go through x
		for(; xi <= xf; xi++){
			i = dy / dx;
			for(j = 0; j < i; j++){
				sht_window->buf[sht_window->bxsize * (yi + j) + xi] = color;
				sheet_refresh(sht_window, xi, yi + j, xi + 1, yi + j + 1, 1);
			}
			yi += i;
			dy -= i;
			dx --;
		}
	}
	return 0;
}

// Print an error string API
static int *api_print_err(char *addr) {
	print_error(addr + program_addr);
	return 0;
}

// Close the window and free the memory API
static int *api_close_window(int sheet) {
	struct SHEET *sht_window;
	io_cli();
	sht_window = (struct SHEET *) sheet;
	sheet_free(sht_window);
	mm_free(sht_window->buf);
	sheet_created = 0;
	io_sti();
	return 0;
}

// Scan a string API
static int *api_scan_str(char *buf, int length) {
	int i = 0, j;
	char ch;
	// If the buffer length is less than 2, scanning string is meaningless
	if(length < 2){
		return 0;
	}
	while(1){
		if(fifo_status(& task_now()->fifo) == 0){
			task_sleep(task_now());
			continue;
		}
		j = fifo_get(& task_now()->fifo);
		ch = key_to_char(j);
		// Get the timer signal and restart the timing
		if(j == TIMER_OFFSET + 1){
			start_timing(1, 50);
		}
		// Illegal charater (Not a character)
		if(ch == 0 || ch == '\b') 
			continue;
		// Enter: return
		if(ch == '\n'){
			*((char *)(program_addr + buf + i)) = 0;
			return 0;
		}
		// Normal characters
		if(i < length - 1){
			*((char *)(program_addr + buf + i)) = ch;
			i++;
			// Return when the buffer is full
			if(i == length - 1){
				*((char *)(program_addr + buf + i)) = 0;
				return 0;
			}
		}
	}
}