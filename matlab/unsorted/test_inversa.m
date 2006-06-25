 n = 360; m = rand(n,n); P = m * m'; 
 
 Q = P;
 G = rand(n,n);
 H = G;
 
 band=30; 
 
 for i=1:n
	 for j=1:n
	 	if abs(i-j)>band 
			Q(i,j)=0; G(i,j)=0;
		end; 
	end; 
end



 tic; inv(P); toc
 tic; inv(Q); toc
 
tic; inv(G); toc

tic; inv(H); toc

