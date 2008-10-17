function plot3bars(x,col1,v_alpha,d,col2,col3)

col1
col2
col3
	hold on;
	plot3(x(1,:),x(2,:),x(3,:),col1);
	
	%% Now project x along plane 
	
	y = repmat(d*v_alpha,1,size(x,2)) + (eye(3)-v_alpha*v_alpha') * x;
	size(y,2)
	
	for i=1:size(y,2)
		plot3( [x(1,i) y(1,i)],[x(2,i) y(2,i)],[x(3,i) y(3,i)],col3);
		plot3( y(1,i),y(2,i), y(3,i),col2);
	
	end
