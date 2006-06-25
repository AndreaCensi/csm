function [dist,white] = test_yasmine3_an(f)

	for i=1:size(f,2)
		dist(:,i) = f{i}.output.x_hat;
		cov = f{i}.output.Cov;
		A = inv(chol(cov))';
		white(:,i) = A * (f{i}.output.x_hat-[1;0;-10.5]);
	end
