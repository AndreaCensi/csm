function result = prepareSampling(set, weights)
	threshold=0.1;
	resolution = min(weights(find(weights>threshold))) / 5
	totWeight = sum(weights);
	spaces = ceil(totWeight/resolution)
	fprintf('prepareSampling: n=%d', spaces);
	setIndex = 1;
	setTotal = weights(1);
	 myTotal = 0;
	
	for i=1:spaces
		result(i) = setIndex;
		
		if myTotal>setTotal  & setIndex < size(weights,2)
			setIndex = setIndex +1 ;
			setTotal = setTotal + weights(setIndex);
		end
		
		myTotal = myTotal + resolution;
		
	end
	
	myTotal
	setTotal
	

