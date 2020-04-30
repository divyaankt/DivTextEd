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

	tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}
int main() {
	enableRawMode();
	
	while(1) {
		char c = '\0';
		read(STDIN_FILENO, &c, 1);
		if (iscntrl(c)) {
			//Block is executed when c is a control character
			//ASCII 0-31 as well 127 are all control characters
			//Without \r, the output shifts rightward constantly at newline
			printf("%d\r\n", c);
		}
		else{
			//Without \r, the output shifts rightward constantly at newline
			printf("%d ('%c')\r\n", c, c);
		}
		if (c == 'q')
			break;
	}

	return 0;
}
