#ifndef SM_LOGGING_H
#define SM_LOGGING_H

extern char * sm_program_name;

void sm_abort(const char *msg, ...);
void sm_debug(const char *msg, ...);
void sm_error(const char *msg, ...);
void sm_info(const char *msg, ...);

#endif
