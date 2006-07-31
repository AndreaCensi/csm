function handle = plotVectors(ref, points, color)
	t = ref(1:2);
	%size(repmat(t,1,size(points,2)));
	points2 =  rot(ref(3)) * points;
	points2 = points2 + repmat(t,1,size(points,2));
	handle = plot(points2(1,:),points2(2,:),color);

