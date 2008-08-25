#include <sys/param.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "csm_all.h"

int sm_debug_write_flag = 0;

const char * sm_program_name = 0;

void sm_debug_write(int flag) {
	sm_debug_write_flag = flag;
}

char sm_program_name_temp[256];
void sm_set_program_name(const char*name) {
	my_basename_no_suffix(name, sm_program_name_temp);
	sm_program_name = sm_program_name_temp;
}

int checked_for_xterm_color = 0;
int xterm_color_available = 0;

void check_for_xterm_color() {
	if(checked_for_xterm_color) return;
	checked_for_xterm_color = 1;
	
	const char * term = getenv("TERM");
	if(!term) term = "unavailable";
	xterm_color_available = !strcmp(term, "xterm-color") || !strcmp(term, "xterm");;
/*	sm_info("Terminal type: '%s', colors: %d\n", term, xterm_color_available); */
}

#define XTERM_COLOR_RED "\e[1;37;41m"
#define XTERM_COLOR_RESET "\e[0m"

#define XTERM_ERROR XTERM_COLOR_RED
#define XTERM_DEBUG "\e[1;35;40m"


void sm_error(const char *msg, ...)
{
	check_for_xterm_color();
	if(xterm_color_available)
		fprintf(stderr, XTERM_ERROR);
		
	va_list ap;
	va_start(ap, msg);
	if(sm_program_name) {
		fputs(sm_program_name, stderr);
		fputs(":err: ", stderr);
	}
	vfprintf(stderr, msg, ap);
	
	if(xterm_color_available)
		fprintf(stderr, XTERM_COLOR_RESET);
}

void sm_info(const char *msg, ...)
{
	check_for_xterm_color();
	
	va_list ap;
	va_start(ap, msg);
	if(sm_program_name) {
		fputs(sm_program_name, stderr);
		fputs(":inf: ", stderr);
	}
	vfprintf(stderr, msg, ap);
}

void sm_debug(const char *msg, ...)
{
	if(!sm_debug_write_flag) return;
	
	check_for_xterm_color();
	
	if(xterm_color_available)
		fprintf(stderr, XTERM_DEBUG);
	
	
	va_list ap;
	va_start(ap, msg);
	if(sm_program_name) {
		fputs(sm_program_name, stderr);
		fputs(":dbg: ", stderr);
	}
	vfprintf(stderr, msg, ap);
	
	
	if(xterm_color_available)
		fprintf(stderr, XTERM_COLOR_RESET);
}
