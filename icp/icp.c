#include "icp.h"
#include "journal.h"

void icp_journal_open(const char* file) {
	journal_open(file);
}