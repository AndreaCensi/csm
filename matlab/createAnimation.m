function createAnimation0(lds, filename, makeVideo)

	horizon = 3;
	fps=5;
	
	% Settings for video
	fig1=figure;
	winsize= get(fig1,'Position');
	winsize(1:2) = [0 0];
	outfile=sprintf('%s',filename);
	mov=avifile(outfile, 'fps', fps, 'quality', 75);
	%set(fig1, 'NextPlot', 'replacechildren');
	        
	set(fig1,'DoubleBuffer','on'); 
	
	n = size(lds,2);
	for i=1:n
		if i>1 & ( vectorNorm2(lds(i).odometry-lds(i-1).odometry) < 0.01 )
			continue
		end
		
		clf
		hold on;
		plotLaserData(lds(i));
		axis('equal');
		hold off
		
		%axis([0 horizon -horizon horizon]); 
		
		if makeVideo
			% Add frame to movie
			mov = addframe(mov,getframe);
		end
		
		% update the screen
		drawnow
	end

	
	% That's all, folks!
	mov = close(mov);
