function s = pv(v)
% prints v=(x,y,theta) as a string, with theta in degrees

if size(v,1)==3
s= sprintf('(%f,%f,%f)', v(1), v(2), rad2deg(v(3)));
end

if size(v,1)==2
s= sprintf('(%f,%f)', v(1), v(2));
end

if size(v,1)==1
s= sprintf('(%f)', v(1));
end

