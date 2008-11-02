#include <math.h>
#include <options/options.h>

#include "../csm/csm_all.h"
#include "../csm/laser_data_drawing.h"

/** Two scans are the same if they have the same timestamp. */
int same_scan(LDP ld1, LDP ld2);
/** Short description for a scan. */
const char * short_desc(LDP ld);

int main(int argc, const char * argv[]) {
	sm_set_program_name(argv[0]);

	const char *in_filename;
	const char *ref_filename;
	const char *out_filename;
	const char *ref_field_string; ld_reference ref_field;
	const char *out_field_string; ld_reference out_field;

	struct option* ops = options_allocate(15);
	options_string(ops, "in", &in_filename, "stdin", "scan matching log");
	options_string(ops, "ref", &ref_filename, "ref.log", "slam log");
	options_string(ops, "out", &out_filename, "stdout", "output file");

	options_string(ops, "ref_field", &ref_field_string, "estimate", "What field to find in ref.");
	options_string(ops, "out_field", &out_field_string, "true_pose", "What field to copy to.");
		
	if(!options_parse_args(ops, argc, argv)) {
		fprintf(stderr, " This program works on two logs: A and B. "
		"For each scan in A, the program searches for the scan in B having the same timestamp. "
		"Then, the true_pose field in B is copied to the scan form A, and it is written to the output.\n");
		options_print_help(ops, stderr);
		return -1;
	}
	
	ref_field = ld_string_to_reference(ref_field_string);
	out_field = ld_string_to_reference(out_field_string);
	
	
	FILE * in_stream  = open_file_for_reading(in_filename);
	FILE * ref_stream = open_file_for_reading(ref_filename);
	FILE * out_stream = open_file_for_writing(out_filename);
	
	if(!in_stream || !ref_stream || !out_stream) return -1;

	LDP ld_in;
	while((ld_in = ld_read_smart(in_stream))) {
		int matched = 0;
		while(1) {
			LDP ld_ref = ld_read_smart(ref_stream);
			if(!ld_ref) break;
			if(same_scan(ld_in, ld_ref)) {
				matched = 1;
				const double *ref_pose = ld_get_reference_pose(ld_ref, ref_field);
				double *out_pose = ld_get_reference_pose_silent(ld_in, out_field);
				copy_d(ref_pose, 3, out_pose);
				ld_write_as_json(ld_in, out_stream);
				fputs("\n", out_stream);
				break;
			}
			ld_free(ld_ref);
		}

		if(!matched) {
			sm_error("Could not match %s. \n", short_desc(ld_in));
			if(feof(ref_stream)) {
				sm_error("..because ref stream has ended.\n");
				break;
			}
			continue;
		}
	
		ld_free(ld_in);
	}
	
	return 0;
}


/** Two scans are the same if they have the same timestamp. */
int same_scan(LDP ld1, LDP ld2) {
	return (ld1->tv.tv_sec == ld2->tv.tv_sec) && 
		(ld1->tv.tv_usec == ld2->tv.tv_usec);
}

char buf[100];
const char * short_desc(LDP ld) {
	sprintf(buf, "LD, tv=%d,%d", (int) ld->tv.tv_sec, (int) ld->tv.tv_usec);
	return buf;
}

