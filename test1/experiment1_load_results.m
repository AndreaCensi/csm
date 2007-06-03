function res = experiment1_load_results(path)
	current_dir = pwd;
	cd(path);
	res.path = path;
	res.scan_map = scan_map;
	res.scan1 = scan1;
	res.scan2 = scan1;
	res.loc_results = loc_results;
	res.sm_results = sm_results;
	cd(current_dir);
	