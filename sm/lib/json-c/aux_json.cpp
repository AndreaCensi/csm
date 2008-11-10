#include <stdlib.h>
#include <vector>
#include <stdio.h>
#include "aux_json.h"
#include "json.h"
#include <string.h>

using namespace std;

/** Reads a file containing a list of json objects*/
int read_jsonlist(const char * filename, vector <JO> *list) {
 	JO tmp;	
 	FILE * input;
	
	if(  (!strcmp(filename, "-"    )) || (!strcmp(filename, "stdin"))  ) { input = stdin;}
	else { input = fopen( filename,"r");}
 	if (input == NULL) {
 		return (0);
 	}
 	while (tmp = json_read_stream(input)) {
 		list->push_back(tmp);
 	}
 	fclose(input);
 	return (1);
}

/** Writes to file a list of json objects. No Append option available*/
int write_jsonlist(const char * filename, vector <JO> *list) {
 	FILE * output;
	
	if		(!strcmp(filename, "-"     )) output = stdout;
	else if	(!strcmp(filename, "stdout")) output = stdout;
	else if	(!strcmp(filename, "stderr")) output = stderr;	
	else {output = fopen(filename, "w");}
	if (output == NULL) {
 		return (0);
 	}
	for (int i = 0; i < (int)list->size(); i++ ) {
 		if (i > 0) fprintf(output,"\n");
		fprintf(output, "%s", json_object_to_json_string(list[0][i]) );
	}
	fclose(output);	
 	return (1);
}
