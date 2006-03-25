function res = normAngle(theta)

while theta>pi
    theta = theta - 2*pi;
end

while theta<-pi
    theta = theta + 2*pi;
end

res = theta;