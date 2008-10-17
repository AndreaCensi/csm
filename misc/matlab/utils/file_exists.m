function res = file_exists(filename)
	fid = fopen(filename);
	if -1 == fid
		res = false;
	else
		res = true;
		fclose(fid);
	end
