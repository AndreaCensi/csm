function plotVectors(ref, points, color)
	t = ref(1:2)
	%size(repmat(t,1,size(points,2)));
	points2 =  rot(ref(3)) * points + repmat(t,1,size(points,2));
	plot(points2(1,:),points2(2,:),color);

