/*** HEADER ***/
#include <ctype.h>
#include <stdio.h>
#include <errno.h>
#include <termios.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/ioctl.h>

/*** DEFINE ***/
//Ctrl key sets Upper 3 bits to zero, more precisely bits 5 and 6
#define CTRL_KEY(k) ((k) & 0x1f)

/*** DATA ***/
//A struct to maintain terminal properties
struct editorConfig {
	int screenRows;
	int screenColumns;
	struct termios orig_termios;
};

struct editorConfig E;

/*** TERMINAL ***/
void die(const char *s) {
	write(STDOUT_FILENO, "\x1b[2J", 4);
  	write(STDOUT_FILENO, "\x1b[H", 3);

	//This function looks at the global errorno set by functions on exiting
	//Based on that it prints a descriptive error message
	//String s is to provide context to the error
	perror(s);
	exit(1);
}

void disableRawMode() {
	if(tcsetattr(STDIN_FILENO, TCSAFLUSH, &E.orig_termios) == -1)
		die("tcsettattr");
}

void enableRawMode() {
	if(tcgetattr(STDIN_FILENO, &E.orig_termios) == -1)
		die("tcgetattr");

	atexit(disableRawMode);

	struct termios raw = E.orig_termios;
	
	//c_iflag is a input flag
	//it disables Ctrl+S and Ctrl+Q
	//Ctrl + M should actually be read as 13, whereas we find it to be 10
	//Terminal is helpfully translating any carriage returns (13, '\r') inputted by the user into newlines (10, '\n').
	//ICRNL fixes this behavious CR-Carriage Return NL-Newline
	//Rest are misc flags
	raw.c_iflag &= ~(ICRNL | IXON | ISTRIP | INPCK | BRKINT);

	//c_oflag is an output flag
	//Terminal translates "\n" to "\r\n", this is required for start of newline
	//turn-off output processing by OPOST flag
	raw.c_oflag &= ~(OPOST);

	//Misc Flag
	raw.c_cflag |= (CS8);

	//c_lflag misc flag
	//ICANON is a flag which is responsible for Canonical Mode
	//ECHO is a bitflag, defined as 00000000000000000000000000001000 it echos character on terminal
	//ISIG disables Ctrl+C and Ctrl+Z
	//IEXTEN disables Ctrl+V, which doesn't usually cause problems on most of systems
	//IEXTEN also disables Ctrl+O, which discards this character on MacOS
	raw.c_lflag &= ~(ECHO | ICANON | ISIG | IEXTEN);

	//The VMIN value sets the minimum number of bytes of input needed before read() can return
	raw.c_cc[VMIN] = 0;
	
	//The VTIME value sets the maximum amount of time to wait before read() returns
	//We set it to 1/10th of a sec or 100 millisecs
	raw.c_cc[VTIME] = 1;

	if(tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1)
		die("tcsetattr");
}

char editorReadKey() {
	int nread;
	char c;
	while((nread = read(STDIN_FILENO, &c, 1)) != 1) {
		if(nread == -1 && errno != EAGAIN)
			die("read");
	}
	return c;
}

int getWindowSize(int *rows, int *cols) {
	struct winsize ws;

	//TIOCSWINSZ stands for Terminal IOCtl (which itself stands for Input/Output Control) 
	//Get WINdow SiZe.)
	if (ioctl(STDOUT_FILENO, TIOCSWINSZ, &ws) == -1 || ws.ws_col == 0) {
		return -1;
	}
	else {
		//This is a common way to return multiple values from a function in C
		*cols = ws.ws_col;
		*rows = ws.ws_row;
		return 0;
	}
}

/*** OUTPUT ***/

void editorDrawRows() {
	int y;
	for (y=0; y<E.screenRows; y++) {
		write(STDOUT_FILENO, "~\r\n", 3);
	}
}
void editorRefreshScreen() {
	//\x1b[2J is a 4-byte escape sequence
	write(STDOUT_FILENO, "\x1b[2J", 4);
	//Position cursor at top-left of the terminal
	write(STDOUT_FILENO, "\x1b[H", 3);

	editorDrawRows();

	//After drawing ~ move to the top-left of screen 
  	write(STDOUT_FILENO, "\x1b[H", 3);
}

/*** INPUT ***/
void editorProcessKeypress() {
	char c = editorReadKey();

	switch(c) {
		case CTRL_KEY('q'):
			write(STDOUT_FILENO, "\x1b[2J", 4);
  			write(STDOUT_FILENO, "\x1b[H", 3);
			exit(0);
		break;	
	}
}

/*** INIT ***/

void initEditor() {
	if(getWindowSize(&E.screenRows, &E.screenColumns) == -1)
		die("getWindowSize");
}
int main() {
	enableRawMode();
	initEditor();

	while(1) {
		editorRefreshScreen();
		editorProcessKeypress();
	}

	return 0;
}
