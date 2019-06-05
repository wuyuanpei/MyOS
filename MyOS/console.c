#include "bootpack.h"

/* (x,y): the current character display place
 (xc, yc): the current cursor display place
 show_cursor: 1: display green, 0: display black
*/
static int x = -1, y = -1, xc = -1, yc = -1, show_cursor = 1;
// The width and height of the window
static int width, height;
// Number of cols and rows that could be displayed
static int cols, rows; 
// CMD, ARGC and ARG bufs
static char *arg_buf[MAX_ARG_BUF];
static int argc;
static char cmd_buf[MAX_CMD_BUF];
// The index that the input will be put into in the cmd_buf
static int index;
// Sheet and buffer
static struct SHEET *sht_window;
static unsigned char *buf_window;
// Static function declarations
static void new_line(char arg);
static void new_char(char ch);
static void parse_cmd(void);
static char str_cmp(char *test, char *target);
static void restore(void);
static void run_cmd(void);
static void print_string(char *str);

/////////////////// Buildin Commands /////////////////////

// c, cls, clear
static void cmd_cls(void){
	int i;
	if(argc != 1){
		print_string("Usage: c/clear/cls");
		return;
	}
    for (i = 0; i < cols; i++) {
		new_line(0);
	}
	x = y = xc = yc = -1;
	// This line will be followed in the while loop in the main method
	//new_line(1);
}

// m, mem, memory
static void cmd_mem(void) {
	char buf[50];
	int total = mm_total() >> 20;
	int use = (mm_total() >> 20) - (mm_check() >> 20);
	int sbrk = mm_sbrk();
	int usage = (use * 10000 + (total >> 1)) / total;//Round
	if(argc != 1){
		print_string("Usage: m/mem/memory");
		return;
	}
	sprintf(buf,"mem :  %4d / %4d MB       usage:  %2d.%02d %%", use, total, usage / 100, usage % 100);
    print_string(buf);
	sprintf(buf,"sbrk:  0x%08x",sbrk);
	print_string(buf);
	sprintf(buf," -u :  %4d MB              -f   :  %4d MB", (mm_total() - sbrk) >> 20, (mm_check() >> 20) - ((mm_total() - mm_sbrk()) >> 20));
	print_string(buf);
}

// ls, dir
static void cmd_ls(void) {
	struct FILEINFO *finfo = (struct FILEINFO *) (ADR_DISK_IMG + 0x002600);
	char buf[30];
	int x, y;
	if(argc != 1){
		print_string("Usage: ls/dir");
		return;
	}
	for (x = 0; x < 224; x++) { // At most 224 files
		if (finfo[x].name[0] == 0x00) { // First character is 0 : No such file
			continue;
		}
		if (finfo[x].name[0] != 0xe5) { // File not deleted
			// Exclude directory and non-file information
			if ((finfo[x].type & 0x18) == 0) {
				int size = (finfo[x].size + 512) >> 10; // Size in KB
				// Less than 1 KB
				if(finfo[x].size < 1024)
					sprintf(buf, "filename.ext %6d B", finfo[x].size);
				else // more than 1 KB
					sprintf(buf, "filename.ext %6d K", size);
				for (y = 0; y < 8; y++) {
					buf[y] = finfo[x].name[y];
				}
				buf[9] = finfo[x].ext[0];
				buf[10] = finfo[x].ext[1];
				buf[11] = finfo[x].ext[2];
				print_string(buf);
			}
		}
	}
}

// type, cat
static void cmd_type(void) {
	// Every time this command is run, we decompress FAT one time and copy
	// the file data into p
	int *fat; // fat buffer
	int index = 0;
	char buf[13] = {0};
	char *p;
	int i, j;
	struct FILEINFO *finfo = (struct FILEINFO *) (ADR_DISK_IMG + 0x002600);
	// Cmd format not correct
	if(argc != 2){
		print_string("Usage: cat/type [filename]");
		return;
	}
	// Turn all characters into lowercase
	while((*(arg_buf[1] + index)) != 0){
		if((*(arg_buf[1] + index)) >= 'a' && (*(arg_buf[1] + index)) <= 'z'){
			(*(arg_buf[1] + index)) -= 0x20;
		}
		index++;
	}
	// Iterate all the entries
	for (i = 0; i < 224; i++) { // At most 224 files
		if (finfo[i].name[0] == 0x00) { // First character is 0 : No such file
			continue;
		}
		if (finfo[i].name[0] != 0xe5) { // File not deleted
			// Exclude directory and non-file information
			if ((finfo[i].type & 0x18) == 0) {
				// Empty the buffer
				for(j = 0; j < 13; j++){
					buf[j] = 0;
				}
				// Copy at most 8 chars
				for (j = 0; j < 8; j++) {
					if(finfo[i].name[j] != ' ')
						buf[j] = finfo[i].name[j];
					else
						break;
				}
				// Fill buf
				if(finfo[i].ext[0] != ' '){
					buf[j] = '.';
					buf[j+1] = finfo[i].ext[0];
				}
				if(finfo[i].ext[1] != ' ')
					buf[j+2] = finfo[i].ext[1];
				if(finfo[i].ext[2] != ' ')
					buf[j+3] = finfo[i].ext[2];
				// Compare the filename.ext
				if(str_cmp(arg_buf[1],buf)){
					// Found
					j = finfo[i].size; // File Size
					fat = (int *)mm_malloc(2880 * 4);
					// Decompress FAT into fat
					file_readfat(fat, (unsigned char *)(ADR_DISK_IMG + 0x200));
					// File buffer
					p = (char *) mm_malloc(j);
					file_loadfile(finfo[i].clustno, j, p, fat, (unsigned char *)(ADR_DISK_IMG + 0x3e00));
					// Printinfo
					new_line(0);
					for(i = 0; i < j; i++){
						if((*(p+i)) == 0x0D) continue; // Carriage Return '\r'
						if((*(p+i)) == 0x0A) { // '\n'
							new_line(0);
							continue;
						}
						if((*(p+i)) == 0x09) { // Tab
							do{
								new_char(' ');
							} while((x % 4) != 0);
							continue;
						}
						new_char(*(p+i));
					}
					mm_free(fat);
					mm_free(p);
					return;
				}
			}
		}
	}
	print_string("File Not Found");
}

static char *buildin_cmd[BUILDIN_CMD_NUM] = {
    "c", "cls", "clear", "m", "mem", "memory",
	"ls", "dir", "type", "cat"
};
static void (*buildin_cmd_addr[BUILDIN_CMD_NUM])(void) = {
	& cmd_cls, & cmd_cls, & cmd_cls, & cmd_mem, & cmd_mem, & cmd_mem,
	& cmd_ls, & cmd_ls, & cmd_type, & cmd_type
};

//////////////////////////////////////////////////////////

// Draw a new line with 1 : drawing '>'; with 0 : nothing
static void new_line(char arg){
	// Next line
	x = 0;
	xc = 0;
	y++;
	yc++;
	if(y == rows){
		y = 0;
		yc = 0;
	}
	// Empty the whole line
	draw_rect(buf_window, width, COL8_BLACK, 3, 22 + 16 * y, width - 4, 37 + 16 * y);
	sheet_refresh(sht_window, 3, 22 + 16 * y, width - 3, 38 + 16 * y, 0);
	// If arg, draw '>'
	if(arg){
		xc++; // xc = 1
		// Draw '>' at (x,y)
		draw_rect(buf_window, width, COL8_BLACK, 3 + 8 * x, 22 + 16 * y, 10 + 8 * x, 37 + 16 * y);
		draw_char(buf_window, width, COL8_GREEN, 3 + 8 * x, 22 + 16 * y, hankaku + '>' * 16);
		sheet_refresh(sht_window, 3 + 8 * x, 22 + 16 * y, 11 + 8 * x, 38 + 16 * y, 0);
		x++; // x = 1
	}
	
}

// Draw the char and move to a new position
static void new_char(char ch){
	// Set the cursor black
	draw_rect(buf_window, width, COL8_BLACK, 3 + 8 * xc, 22 + 16 * yc, 10 + 8 * xc, 37 + 16 * yc);
	sheet_refresh(sht_window, 3 + 8 * xc, 22 + 16 * yc, 11 + 8 * xc, 38 + 16 * yc, 0);
	// Cursor moves to the next position
	xc++;
	// Draw the character at (x,y)
	draw_char(buf_window, width, COL8_GREEN, 3 + 8 * x, 22 + 16 * y, hankaku + ch * 16);
	sheet_refresh(sht_window, 3 + 8 * x, 22 + 16 * y, 11 + 8 * x, 38 + 16 * y, 0);
	// Next position
	x++;
	// x reaches boundary
	if(x == cols){
		new_line(0);
	}
}

// New a line and draw a string
static void print_string(char *str) {
	new_line(0);
	while(*str){
		new_char(*(str++));
	}
}

// Parse/Truncate the cmd and put the starting address of every arg into buf
static void parse_cmd(void){
	char *cmd = cmd_buf;
	char **buf = arg_buf;
    char count_buf = 0;
    // Get the cmds
    while(count_buf < MAX_ARG_BUF){
        // Move to the first non-space character
        while(*cmd == ' '){
            *cmd = 0;
            cmd++;
        }
        // If we reach the end of the line without any argument
        if(*cmd == '\n' || *cmd == 0) {
                return;
        }
        // Set the char pointer
        buf[count_buf] = cmd;
		argc++;
        // Move to the first space character
        while(*cmd != ' '){
            // If we reach the end of the line immediately after an argument
            if(*cmd == '\n' || *cmd == 0) {
                *cmd = 0;
                return;
            }
            cmd++;
        }
        // next buffer
        count_buf++;
    }
    // Set an end tag
    *cmd = 0;
}

/* Compare two string, if same return 1, if different return 0
   Returning time depends on the length of target */
static char str_cmp(char *test, char *target){
	char res = 1;
	while(*target){
		if(*test != *target){
			res = 0;
		}
		target++;
		test++;
	}
	if(*test != 0)
		res = 0;
	return res;
}

/* Restore the values of index, arg_buf[MAX_ARG_BUF], and cmd_buf[MAX_CMD_BUF]
*/
static void restore(void) {
	index = 0;
	argc = 0;
	int i;
	for (i = 0; i < MAX_ARG_BUF; i++)
	{
		arg_buf[i] = 0;
	}
	for (i = 0; i < MAX_CMD_BUF; i++)
	{
		cmd_buf[i] = 0;
	}
}

// Run the command based on arg_buf[0] and buildin_cmd[i]
static void run_cmd(void) {
    char *cmd = arg_buf[0];
	if(argc == 0) return; // Empty
	int i;
    for(i = 0; i < BUILDIN_CMD_NUM; i++) {
		if(str_cmp(cmd, buildin_cmd[i])) {
			// Run the buildin command in the same task
			(*(buildin_cmd_addr + i))();
			return;
		}
    }
	print_string("Command Not Found");
}

// CMD task Main method
void task_console_main(void)
{
	// Initialize window data
	width = 406;
	height = 345;
	cols = (width - 6) >> 3; // Minus the margin and then / 8
	rows = (height - 25) >> 4; // Minus the margin and then / 16
	// Initialize sheet and buffer
	sht_window = sheet_alloc();
	buf_window = mm_malloc(width * height); // window size
	sheet_setbuf(sht_window, buf_window, width, height, 0xff); // 0xff for col_inv
	make_window(buf_window, width, height, "Command", 1);
	// Black background
	draw_rect(buf_window, width, COL8_BLACK, 3, 22, width - 4, height - 4); // Leave margin
	sheet_updown(sht_window, 1);
	sheet_slide(sht_window, 5, 70); // Position on the screen
	// Timer for the cursor (Green->Black, B->G)
	start_timing(1, 50);
	// Restore the buffers and the index pointer
	restore();
	// Draw a new line
	new_line(1);
	while(1){
		if(fifo_status(& task_now()->fifo) == 0){
			task_sleep(task_now());
			continue;
		}
		unsigned int data = fifo_get(& task_now()->fifo);
		// Get the timer signal
		if(data == TIMER_OFFSET + 1){
			start_timing(1, 50);
			// Display the cursor
			if(show_cursor){
				draw_rect(buf_window, width, COL8_YELLOW, 3 + 8 * xc, 22 + 16 * yc, 10 + 8 * xc, 37 + 16 * yc);
				sheet_refresh(sht_window, 3 + 8 * xc, 22 + 16 * yc, 11 + 8 * xc, 38 + 16 * yc, 0);
			}else{
				draw_rect(buf_window, width, COL8_BLACK, 3 + 8 * xc, 22 + 16 * yc, 10 + 8 * xc, 37 + 16 * yc);
				sheet_refresh(sht_window, 3 + 8 * xc, 22 + 16 * yc, 11 + 8 * xc, 38 + 16 * yc, 0);
			}
			// Switch the state of the cursor
			show_cursor ^= 1;
			continue;
		}

		char ch = key_to_char(data);
		// Illegal charater (Not a character)
		if(ch == 0) 
			continue;
		// Enter
		if(ch == '\n'){
			// If no input, continue
			if(index == 0) continue;
			// Set the cursor black
			draw_rect(buf_window, width, COL8_BLACK, 3 + 8 * xc, 22 + 16 * yc, 10 + 8 * xc, 37 + 16 * yc);
			sheet_refresh(sht_window, 3 + 8 * xc, 22 + 16 * yc, 11 + 8 * xc, 38 + 16 * yc, 0);
			// Parse and run the command
			parse_cmd();
			run_cmd();
			// Restore the buffer and index
			restore();
			// Draw a new line
			new_line(1);
			continue;
		}
		// Backspace
		if(ch == '\b'){
			// Nothing to backspace
			if(index == 0) continue;
			// Set the cursor black
			draw_rect(buf_window, width, COL8_BLACK, 3 + 8 * xc, 22 + 16 * yc, 10 + 8 * xc, 37 + 16 * yc);
			sheet_refresh(sht_window, 3 + 8 * xc, 22 + 16 * yc, 11 + 8 * xc, 38 + 16 * yc, 0);
			// Go back one space
			x--;
			if(x == -1){
				y--;
				if(y == -1){
					y = rows - 1;
				}
				x = cols - 1;
			}
			// Set the character black
			draw_rect(buf_window, width, COL8_BLACK, 3 + 8 * x, 22 + 16 * y, 10 + 8 * x, 37 + 16 * y);
			sheet_refresh(sht_window, 3 + 8 * x, 22 + 16 * y, 11 + 8 * x, 38 + 16 * y, 0);
			if(xc != 0)
				xc--;
			else {
				yc--;
				if(yc == -1){
					yc = rows - 1;
				}
				xc = cols - 1; 
			}
			cmd_buf[--index] = 0;
			continue;
		}
		
		if(index < MAX_CMD_BUF - 1){
			// Display the character
			new_char(ch);
			// Put the character in the cmd_buf
			cmd_buf[index++] = ch;
		}
	}
}

