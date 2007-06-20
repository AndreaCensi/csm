theta = deg2rad(4);
t = [0.3; -0.2];

p = [1 0; 0 1; -1 0; 2 1; 4 2]';
alpha = [deg2rad(0);deg2rad(10);deg2rad(20);deg2rad(50);deg2rad(-20);];

noise = 0
for i=1:size(p,2)
	corr{i}.p = p(:,i);
	corr{i}.q = (rot(theta)*p(:,i)+t) + randn(2,1)*noise;
	corr{i}.C = vers(alpha(i))*vers(alpha(i))';
%	corr{i}.C = eye(2);
end

res = general_minimization(corr);

res.x


