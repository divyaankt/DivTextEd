#include <ctype.h>
#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <stdlib.h>

struct termios orig_termios;

void disableRawMode() {
	tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}
void enableRawMode() {
	tcgetattr(STDIN_FILENO, &orig_termios);
	atexit(disableRawMode);

	struct termios raw = orig_termios;
	
	//c_iflag is a input flag
	//it disables Ctrl+S and Ctrl+Q
	raw.c_iflag &= ~(IXON);

	//c_lflag misc flag
	//ICANON is a flag which is responsible for Canonical Mode
	//ECHO is a bitflag, defined as 00000000000000000000000000001000 
	//ISIG disables Ctrl+C and Ctrl+Z
	//IEXTEN disables Ctrl+V, which doesn't usually cause problems on most of systems
	//IEXTEN also disables Ctrl+O, which discards this character on MacOS
	raw.c_lflag &= ~(ECHO | ICANON | ISIG | IEXTEN);

	tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}
int main() {
	enableRawMode();
	char c;

	while(read(STDIN_FILENO, &c, 1) == 1 && c != 'q') {
		if (iscntrl(c)) {
			//Block is executed when c is a control character
			//ASCII 0-31 as well 127 are all control characters
			printf("%d\n", c);
		}
		else{
			printf("%d ('%c')\n", c, c);
		}
	}

	return 0;
}
