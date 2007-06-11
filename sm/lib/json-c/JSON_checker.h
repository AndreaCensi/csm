/* JSON_checker.h */

extern int JSON_checker(unsigned short p[], int length);
extern int JSON_checker_at_character(void);


void JSON_checker_init(void);
int JSON_checker_push(int b);
int JSON_checker_finished(void);

