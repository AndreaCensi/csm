
#include <stdlib.h>
#include <vector>
#include <stdio.h>
#include "json.h"

using namespace std;

/** Reads a file containing a list of json objects*/
int read_jsonlist(const char * filename, vector <JO> *list);

/** Writes to file a list of json objects. No Append option available*/
int write_jsonlist(const char * filename, vector <JO> *list);
