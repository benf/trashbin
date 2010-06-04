#include <unistd.h>
#include <signal.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>

#include <locale.h>
#include <wchar.h>
#include <wctype.h>
#include <ctype.h>

#include <termios.h>

#include <curses.h>
#include <term.h>

#define max(a, b) ((a) > (b) ? (a) : (b))

static struct termios term, term_bak;

static inline void test(int _test, char *error) {
	if (!_test)
		return;
	fprintf(stderr, "error: %s\n", error);
	exit(EXIT_FAILURE);
}

#ifdef USE_CURSES
struct keys {
	char *capname, *name;
} keys[] = {
	{"kcuu1", "UP"},
	{"kcud1", "DOWN"},
	{"kcub1", "LEFT"},
	{"kcuf1", "RIGHT"},
	{"khome", "HOME"},
	{"kend",  "END"},
	{"knp",   "PG-DOWN"},
	{"kpp",   "PG-UP"},
	{"kbs",   "BCKSPAC"},
	{"kdch1", "DEL"},
	{"kich1", "INS"}
};

/*
 * ret: < 0   <--> input is not an F_key
 *      >=0   <--> input is F_ret
 */
int is_F_key(char *input, size_t len) {
	int key;
	char str[8];
	char *capname;
	for (key = 0; key < 64; ++key) {
		snprintf(str, 8, "kf%d", key);
		capname = tigetstr(str);
		/* capname is absent or incorrect (2. should never happen (: ) */
		if ((intptr_t)capname <= 0)
			continue;
		if (strlen(capname) != len) /* if lengths differ, they cant be equal */
			continue;
		if (strncmp(input, capname, len) == 0)
			return key;
	}
	return -1; /* <0 indicates key does not match */
}

/*
 * ret: NULL    <--> key could  not resolved
 * 	!= NULL <--> char * to resolved key name
 */
char *print_cntrl_key(char *input, size_t len) {
	const size_t size = sizeof(keys)/sizeof(struct keys);
	int i;
	char *capname;
	static char str[8]; /* static because we want to return this */
	for (i = 0; i < size; ++i) {
		capname = tigetstr(keys[i].capname);
		if ((intptr_t)capname <= 0)
			continue;
		if (strlen(capname) != len)
			continue;
		if (strncmp(capname, input, len) == 0)
			return keys[i].name;

	}
	
	if ((i = is_F_key(input, len)) >= 0) {
		snprintf(str, 8, "F%d", i);
		return str;
	}
	return NULL;
}

static inline char *color(int num) {
	char *str = tigetstr("setaf");
	if ((intptr_t)str  == 0)
		return "";
	else
		return tparm(str, num);
}

static inline void reset_color(void) {
	char *str = tigetstr("sgr0");
	if ((intptr_t)str > 0)
		putp(str);
	fflush(stdout);
}
#else /* not USE_CURSES */
static inline void reset_color(void) {}
static inline char *color(int num) { return ""; } 

// need to use a macro here (putp was already defined in curses.h)
#define putp(str) printf("%s", (str));
#endif /* USE_CURSES */

void restore_term(void) {
	reset_color();
	test(tcsetattr(STDIN_FILENO, TCSANOW, &term_bak) == -1, strerror(errno));
}

int main(void) {
	ssize_t n;
	int i;
	wchar_t wide_char;
#ifdef USE_CURSES
	test(setupterm(NULL, STDOUT_FILENO, NULL) == ERR, "could not inizialize curses");
#else
	printf("\n"
		"!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n"
		"! PLEASE APPEND '-l curses -D USE_CURSES' on to the gcc !\n"
		"! cmd to have colored output and resolved escape codes! !\n"
		"!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n\n");
#endif
	mbstate_t *mbstate  = &(mbstate_t) {0};
	test(mbsinit(mbstate) == 0, "mbstate is not in an initial state");

	setlocale(LC_CTYPE, "");
	const size_t maxkey = max(5, MB_CUR_MAX);
	char *buf = calloc(maxkey, sizeof(char));
	test(buf == NULL, "cannot allocate memory");

	test(tcgetattr(STDIN_FILENO, &term_bak) == -1, strerror(errno));
	term = term_bak;
	term.c_lflag &= ~(ICANON | ECHO);

	test(atexit(restore_term) != 0, "cannot set exit function");

	test(signal(SIGINT,  exit)    == SIG_ERR, "cannot set SIGINT handler");
	test(signal(SIGTSTP, SIG_IGN) == SIG_ERR, "cannot set SIGTSTP handler");

	/* disable printf (etc) buffering */
	if (setvbuf(stdout, NULL, _IONBF, 0) != 0) clearerr(stdout);

	printf("%sCurrent Terminal Settings:\n", color(COLOR_YELLOW));
	reset_color();

	system("stty -a | sed \"s/.\\(icanon\\|echo\\) /$(tput setaf 1)\\0$(tput sgr0)/g\"");

	test(tcsetattr(STDIN_FILENO, TCSANOW, &term) == -1, strerror(errno));

	printf("\n%sNew Terminal Settings:\n", color(COLOR_YELLOW));
	reset_color();
	system("stty -a | sed \"s/.\\(icanon\\|echo\\) /$(tput setaf 2)\\0$(tput sgr0)/g\"");

	printf("\n%sMultibyte current maximum: ", color(COLOR_GREEN));
	printf("%s%zu\n", color(COLOR_MAGENTA), MB_CUR_MAX);

	printf("%sSet maximum key size to:   ", color(COLOR_GREEN));
	printf("%s%zu\n", color(COLOR_MAGENTA), maxkey);

	reset_color();
	putchar('\n');
	do {
		n = read(STDIN_FILENO, buf, maxkey);

		test(n == -1, strerror(errno));
		if (n == 0) continue; /* what ever happend its not what we want */

		/* do a multibyte to wide_char conversion (i.e. utf8->utf32 if using utf8 locale) */
		switch (mbrtowc(&wide_char, buf, n, mbstate)) {
		case -2: /* couldnt parse complete multibyte - shouldnt happend, see maxkey */
			//wide_char = 0;
			continue;
			break;
		case -1: /* illegal multibyte sequence - lets ignore */
			printf("error: %s\n", strerror(errno));
			errno = 0;
			continue;
		}

		if (buf[0] == '\0') {
			printf("%s\\0", color(COLOR_RED));
		} else if (buf[0] == ' ') {
			printf("%sSPACE", color(COLOR_RED));
		} else if (iscntrl(buf[0])) {
			putp(color(COLOR_RED));

#ifdef USE_CURSES
			char *str;
			if ((str = print_cntrl_key(buf, n)) != NULL)
				printf("%s", str);
			else
#else
			printf("attention: curses is disabled, so escape codes arent resolved!\n");

#endif
			switch(buf[0]) {
			case 0x1B: 
				if (n == 1) {
					fputs("\\e", stdout);
				} else {
					printf("n.d.");
				}
				
				break;
			case '\t': 
				fputs("\\t", stdout); break;
			case '\n':
				fputs("\\n", stdout); break;
			case '\r':
				fputs("\\r", stdout); break;
			case '\v':
				fputs("\\v", stdout); break;
			case '\f':
				fputs("\\f", stdout); break;

			default:
				// display ^KEY
				putchar('^');
				putchar('A' -1 + buf[0]);
			}
		} else if (iswprint(wide_char)) {
			putp(color(COLOR_YELLOW));
			write(STDOUT_FILENO, buf, n);
		} else {
			printf("%sn.d.", color(COLOR_RED));
		}

		printf("\t%s%zd\t", color(COLOR_MAGENTA), n);

		const short decimal_width = 4; /* beinhaltet führendes leerzeichen, bsp: " 123" */
		putp(color(COLOR_GREEN));
		for (i = 0; i < n; ++i)
			printf("%*hhu", decimal_width, buf[i]);
		if (n < maxkey)
			printf("%*c", (int) (maxkey - n) * decimal_width, ' ');


		printf("  %s", color(COLOR_CYAN));
		for (i = 0; i < n; ++i)
			printf("0x%02hhx ", buf[i]);

		putchar('\n');

	} while (buf[0] != '@');

	free(buf);
	return EXIT_SUCCESS;
}
