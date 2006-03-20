function sd = test_est_deriv_log(k)

sigma=0.02;

if true
	S=load('logs/bighouse_half.mat');
	log = S.log_bighouse_half;
	sd = log{k}
	sd.readings = sd.readings + sigma * randn(1,sd.nrays);
	sd.points = [sd.readings .* cos(sd.theta);  sd.readings .* sin(sd.theta)]; 
end



%sd = straight(150, pi*0.5, 5, sigma);
sd = circ(150, pi*0.9, 5, sigma);


var = sigma^2;
curv = 10;
maxDist = 0.15;


sd = computeSurfaceNormals_sound(sd, var, curv, maxDist);

f = figure;
params.plotNormals = true;
plotLaserData(sd,params);
axis('equal');

f = figure;
subplot(3,1,1);
hold on
U = 3*rad2deg(sqrt(sd.alpha_error));
errorbar(rad2deg(sd.theta),rad2deg(sd.alpha),U,U,'k.');
plot(rad2deg(sd.theta),rad2deg(sd.ar_alpha),'g.');

subplot(3,1,2);

plot(rad2deg(sd.theta),U/3);

subplot(3,1,3);

plot(rad2deg(sd.theta),rad2deg(sd.alpha-sd.true_alpha)./U);

indexes=10:140;
writeStats(sd.alpha(indexes),sd.true_alpha(indexes),sd.alpha_error(indexes));

function writeStats(x,tr,var) 
	% percentuale entro 95%
	white = (x-tr) ./ sqrt(var);
	for p=1:3
		perc = sum( abs(white) < p ) / size(white,2);
		fprintf('Perc: %d %f\n',p,perc);
	end
	




function ld=straight(nrays,fov, dist, sigma)

	ld.nrays=nrays;

	for i=1:nrays
		ld.theta(i) = -fov/2 + fov * i / nrays;
		ld.readings(i) = dist / cos(ld.theta(i));
		ld.readings(i) = ld.readings(i) + sigma * randn(1,1);
		ld.points(:,i) = [cos(ld.theta(i)) sin(ld.theta(i))]' * ld.readings(i);
		ld.true_alpha(i) = 0; 
	end

function ld=circ(nrays,fov, dist, sigma)

	ld.nrays=nrays;

	for i=1:nrays
		ld.theta(i) = -fov/2 + fov * i / nrays;
		ld.readings(i) = dist;
		ld.readings(i) = ld.readings(i) + sigma * randn(1,1);
		ld.points(:,i) = [cos(ld.theta(i)) sin(ld.theta(i))]' * ld.readings(i);
		ld.true_alpha(i) = ld.theta(i); 
	end

