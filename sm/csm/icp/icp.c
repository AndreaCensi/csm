#include "../sm.h"
#include "../journal.h"

void sm_journal_open(const char* file) {
	journal_open(file);
}

