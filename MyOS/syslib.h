/* System API header file */
// Macros
// types
#define SHEET int
#define COLOR char

// values
/* Colors on the screen */
#define COL_BLACK		0
#define COL_RED			1
#define COL_GREEN		2
#define COL_YELLOW		3
#define COL_BLUE		4
#define COL_PURPLE		5
#define COL_CYAN		6
#define COL_WHITE		7
#define COL_GRAY		8
#define COL_D_RED		9
#define COL_D_GREEN		10
#define COL_D_YELLOW		11
#define COL_D_BLUE			12
#define COL_D_PURPLE		13
#define COL_D_CYAN			14
#define COL_D_GRAY			15
#define COL_TRANSPARENT		0xff
/* functions defined in syslib_asm.nas */
//void end(void); // End the application
void print_int(int); // Print an integer
void print_str(char *); // Print a string
SHEET new_window(int width, int height, int x, int y, char *title); // Draw a window
void draw_str(SHEET sheet, COLOR color, char *str, int x, int y); // Draw a string on the window
void draw_rec(SHEET sheet, COLOR color, int xi, int yi, int xf, int yf); // Draw a rectangle on the window
void *space(void); // Return the starting address of .space segment
void draw_pt(SHEET sheet, COLOR color, int x, int y); // Draw a point on the window
void draw_line(SHEET sheet, COLOR color, int xi, int yi, int xf, int yf); // Draw a line on the window
void print_err(char *); // Print an error string
void close_window(SHEET sheet); // Close the window
void scan_str(char *buf, int length); // Wait for a string input, with maximum buffer length; only return when the buffer is full or enter is typed
