#include "raytracer.h"

struct raytracer_params {
	const char * file_map;
	const char * file_poses;
	const char * file_output;
	double fov_deg;
	int nrays;
	double max_reading;
};


bool load_env_from_json(Environment& env, JO jo);
double cosine_between_angles(double a1, double a2);


int main(int argc, const char** argv)
{
	sm_set_program_name(argv[0]);
	struct raytracer_params p;
	
	struct option* ops = options_allocate(60);
	options_string(ops, "map", &p.file_map, "map.json", "Environment description");
	options_string(ops, "poses", &p.file_poses, "-", "List of poses");
	options_string(ops, "out", &p.file_output, "stdout", "Output file ");
	options_int(ops, "nrays", &p.nrays, 181,  "Number of rays");
	options_double(ops, "fov_deg", &p.fov_deg, 180.0,  "Field of view (degrees)");
	options_double(ops, "max_reading", &p.max_reading, 80.0,  "Sensor horizon (meters)");
	
	if(!options_parse_args(ops, argc, argv)) {
		sm_info(" simulats one scan from map. \n\nUsage:\n");
		options_print_help(ops, stderr);
		return -1;
	}
	
	
	FILE * file_map = open_file_for_reading(p.file_map);
	if(!file_map) return -2;
	
	JO jo_map = json_read_stream(file_map); 
	if(!jo_map) {
		sm_error("Could not read JSON from file '%s'\n", p.file_map);
		return -3;
	}
	if(json_read_stream(file_map)) {
		sm_error("I expect only 1 JSON object in file '%s'\n", p.file_map);
		return -4;
	}

	Environment env;
	
	load_env_from_json(env, jo_map);
	
	
	FILE * file_output = open_file_for_writing(p.file_output);
	if(!file_output) return -1;


	FILE * file_poses = open_file_for_reading(p.file_poses);
	if(!file_poses) return -2;
	
	JO jo_pose; int num = 0;
	while( (jo_pose = json_read_stream(file_poses)) )
	{
		num++;
	
		LDP ld = ld_alloc_new(p.nrays);

		if(!jo_read_from_double_array (jo_pose, ld->true_pose, 3, GSL_NAN) || any_nan(ld->true_pose, 3)) {
			sm_error("Bad pose: '%s'.\n", jo_to_string(jo_pose));
			return -5;
		}
		
		
		double fov = deg2rad(p.fov_deg);
		
		ld->min_theta = -fov/2;
		ld->max_theta = +fov/2;
		
		for(int i=0;i<ld->nrays;i++) {
			ld->theta[i] = - fov/2 + fov * i / (ld->nrays-1);;
			
			double rho, alpha; int stuff_id;
			if(env.ray_tracing(ld->true_pose, ld->theta[i] + ld->true_pose[2], rho, alpha, &stuff_id) && (rho<p.max_reading)) {
			
				ld->valid[i] = 1;
				ld->readings[i] = rho;
				
				double relative_alpha = alpha-ld->true_pose[2];

				/* Make sure alpha points out of the wall */
				if( cosine_between_angles(relative_alpha, ld->theta[i]) > 0) {
					relative_alpha += M_PI;
				}
				
				ld->true_alpha[i] = normalize_0_2PI(relative_alpha);

			} else {
				ld->valid[i] = 0;
			} 
			
		}
		
		ld->tv.tv_sec = num;


		JO jo = ld_to_json(ld);
		fputs(jo_to_string(jo), file_output);
		fputs("\n", file_output);
		jo_free(jo);

		ld_free(ld);
	}	

	if(num == 0) {
		sm_error("It looks like there wasn't any pose in the file '%s'\n", p.file_poses);
		return -4;
	}

	jo_free(jo_map);
}



#define jo_expect_array(a) (a!=0 && json_object_is_type(a, json_type_array))
#define jo_expect_object(a) (a!=0 &&  json_object_is_type(a, json_type_object))
#define jo_expect_string(a) (a!=0 && json_object_is_type(a, json_type_string))
#define jo_expect_array_size(a,n) ( (a!=0) && (json_object_is_type(a, json_type_array)&& (jo_array_length(a)==n)))
#define jo_expect_array_size_min(a,n) ( (a!=0) && (json_object_is_type(a, json_type_array)&& (jo_array_length(a)>=n)))


#define expect(a) if(!a) { \
		sm_error("Invalid format: \n\t %s \n", #a); \
			return false; \
		}

#define expect_s(a, s) if(!a) { \
		sm_error("Invalid format: %s \n\t %s \n", s, #a); \
			return false; \
		}
		
double cosine_between_angles(double a1, double a2) {
	return cos(a1)*cos(a2)+sin(a1)*sin(a2);
}


bool load_env_from_json(Environment& env, JO jo_map) {
	jo_expect_object(jo_map);
	
	JO jo_objects = json_object_object_get(jo_map, "objects");
		expect(jo_expect_array(jo_objects));
	
	for(int i=0; i < jo_array_length(jo_objects); i++) {
		JO jo = jo_array_get(jo_objects, i);
			expect(jo_expect_object(jo));
		
		JO jo_type = jo_get(jo, "type"); 
			expect(jo_expect_string(jo_type));
			
		if(!strcmp(jo_get_string(jo_type), "line")) {
			
			JO points = jo_get(jo, "points");
			expect(jo_expect_array_size_min(points, 2));
			
			
			for(int p=0;p<jo_array_length(points)-1;p++) {
				Segment * s = new Segment();
				expect(jo_read_from_double_array (jo_array_get(points, p  ), s->p0, 2, NAN));
				expect(jo_read_from_double_array (jo_array_get(points, p+1), s->p1, 2, NAN));
				env.stuff.push_back(s);
			}

		} else if(!strcmp(jo_get_string(jo_type), "square")) {
				
				JO corners = jo_get(jo, "corners");
				expect_s(jo_expect_array_size(corners, 2), jo_to_string(corners));

				double pmin[2],pmax[2];

				expect(jo_read_from_double_array (jo_array_get(corners, 0  ), pmin, 2, NAN));
				expect(jo_read_from_double_array (jo_array_get(corners, 1), pmax, 2, NAN));

				env.stuff.push_back(new Segment(pmin[0],pmin[1],pmax[0],pmin[1]));
				env.stuff.push_back(new Segment(pmax[0],pmax[1],pmax[0],pmin[1]));
				env.stuff.push_back(new Segment(pmax[0],pmax[1],pmin[0],pmax[1]));
				env.stuff.push_back(new Segment(pmin[0],pmin[1],pmin[0],pmax[1]));

		} else {
			sm_error("unknown object type: '%s'\n", jo_get_string(jo_type));
			return false;
		}
	}
	
	return true;
}


