#include <termios.h>
#include <unistd.h>

void enablerawMode() {
	struct termios raw;

	tcgettattr(STDIN_FILENO, &raw);

	raw.c_lflag &= ~(ECHO);

	tcgettattr(STDIN_FILENO, TCSAFLUSH, &raw);
}
int main() {

	char c;

	while(read(STDIN_FILENO, &c, 1) == 1 && c != 'q');

	return 0;
}
