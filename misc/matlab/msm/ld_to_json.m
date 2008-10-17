function s = ld_to_json(ld, pretty)

	ld.nrays = uint16(ld.nrays);
	ld.valid = uint16(ld.valid);
	s = matlab_to_json(ld, pretty);
	
	
	
