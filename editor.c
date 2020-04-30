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
	//c_lflag misc flag
	//ECHO is a bitflag, defined as 00000000000000000000000000001000 
	raw.c_lflag &= ~(ECHO | ICANON);

	tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}
int main() {
	enableRawMode();
	char c;

	while(read(STDIN_FILENO, &c, 1) == 1 && c != 'q');

	return 0;
}
