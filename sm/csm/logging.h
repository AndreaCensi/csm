#ifndef SM_LOGGING_H
#define SM_LOGGING_H


extern void sm_abort(const char *msg, ...);
extern void sm_debug(const char *msg, ...);
extern void sm_error(const char *msg, ...);
extern void sm_info(const char *msg, ...);

#endif
