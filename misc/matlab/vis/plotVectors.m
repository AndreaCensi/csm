
function handle = plotVectors(ref, points, color)
	% TODO: add check on dimensions
	t = ref(1:2);
	points2 = rot(ref(3)) * points;
	
	%size(points)
	%t
	%repmat(t,1,size(points,2))
	%size(repmat(t,1,size(points,2)))
	%size(points2)
	
	points2 = points2 + repmat(t,1,size(points,2));
	handle = plot(points2(1,:),points2(2,:),color);

