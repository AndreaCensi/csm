function doScanMatch(log)

params.maxAngularCorrectionDeg = 25; % degrees
params.maxLinearCorrection = 0.15; % meters
params.emXYsigma = 0.2;

f=figure;
	set(f,'DoubleBuffer','on'); 
	set(f, 'NextPlot', 'replacechildren');
	
k=1;
for i=2:size(log,2)
	if i>1 & ( vectorNorm2(log(i).odometry-log(i-1).odometry) < 0.01 )
		continue
	end
		
	params.laserData1 = log(i-1);
	params.laserData2 = log(i);

	res(k) = gpm(params); 

		clf;
	subplot(3,1,1)
		hold off;
		plotLaserData(log(i));
		axis('equal');

	subplot(3,1,2);
		hold off;
		l = params.maxLinearCorrection;
		axis([-l l -l l]);
		plot( res(k).T(1,:),res(k).T(2,:), 'b.');
		axis('equal');
		
	subplot(3,1,3);
		hold off;
		l = params.maxLinearCorrection;
		axis([-l l -l l]);
		plotGPM1(res(k));
		axis('equal');
		
		
		k=k+1;
	drawnow;
end
