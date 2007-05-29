function res = display_cov(cov)

	std_x = sqrt(cov(1,1));
	std_y = sqrt(cov(2,2));
	std_th = sqrt(cov(3,3));
	rho_xy = cov(1,2) / (std_x * std_y);
	rho_xth = cov(1,3) / (std_x * std_th);
	rho_yth = cov(2,3) / (std_y * std_th);
	
	res = [std_x, std_y, std_th, rho_xy, rho_xth, rho_yth];