require 'rsm'

class LaserData
	# Computes alpha for each point
	def compute_orientation(neighbourhood, sigma)
		for i in 0..nrays-1

			if not valid? i
				@alpha[i] = GSL::NAN
				@alpha_valid[i]= false
				@cov_alpha[i] = GSL::NAN
				next
			end
			
			# list of index
			neighbours = find_neighbours(i,neighbourhood)
			
#			puts "i = #{i} neigbours = #{neighbours.join(', ')}"

			if neighbours.size == 0
				@alpha_valid[i]= false
				@alpha[i] = GSL::NAN
				@cov_alpha[i] = GSL::NAN
				next
			end
			
#			puts "neighbours for i=#{i} = #{neighbours.join(', ')}"
			thetas = neighbours.map{|j| @theta[j]}
			readings = neighbours.map{|j| @readings[j]}

			@alpha[i], @cov_alpha[i] = filter_orientation(@theta[i],@readings[i],thetas,readings,1);
			@cov_alpha[i]  *= sigma**2

#			puts "------- i = #{i} alpha = #{}"
			@alpha_valid[i] = @alpha[i].nan? ? false : true
			if @alpha[i].nan?
#				puts "We have a problem.."
			end
		end
	end
	
	def filter_orientation(theta0, rho0, thetas, rhos, curv)
		n = thetas.size
		# y = l x + r epsilon
		y = Matrix.alloc(n, 1)
		l = Matrix.alloc(n, 1)
		r = Matrix.alloc(n, n+1)
		
		r.set_all(0.0)
		l.set_all(1.0);
		
		for i in 0..n-1
		#	puts "theta0 = #{theta0}  thetas[i] = #{thetas[i]}"
			y[i,0]   = (rhos[i]-rho0)/(thetas[i]-theta0)
			r[i,0]   = -1/(thetas[i]-theta0);
			r[i,i+1] =  1/(thetas[i]-theta0);
		end
		
#		puts "y = \n#{y}"
#		puts "l = \n#{l}"
#		puts "r = \n#{r}"
		
		bigR = r*r.trans;
		# x = (l^t R^-1 l)^-1 l^t R^-1 y
		cov_f1 = ((l.trans * bigR.inv * l).inv)[0,0]
		f1 = (cov_f1 * l.trans * bigR.inv * y)[0,0]
	
#		puts "f1 = #{f1}"
		#alpha = theta0 + PI/2 + Math.atan(rho0/f1)
		alpha = theta0 - Math.atan(f1/rho0)
		
		
		dalpha_df1  = rho0 / (rho0**2 + f1**2)
		dalpha_drho = -f1 / (rho0**2 + f1**2)
		cov0_alpha = (dalpha_df1**2) * cov_f1 + (dalpha_drho**2)


#		puts " cov_f1 = #{cov_f1} dalpha_df1 #{dalpha_df1**2} dalpha_drho #{dalpha_drho**2} "+
#			" cov0_alpha = #{cov0_alpha}"
#		puts "sotto = #{(rho0**2 + f1**2)}"
			
		if cos(alpha)*cos(theta0)+sin(alpha)*sin(theta0)>0
			alpha = alpha + PI
		end
		
		return alpha, cov0_alpha
	end
	
	# uses params[:gpm_neighbours]?
	def find_neighbours(i, neighbourhood)
		up = i; 
		while (up+1 <= i+neighbourhood) and (up+1<nrays) and (valid? up+1) and 
				(cluster[up+1] == cluster[i])
			up+=1; 
		end
		down = i; 
		while (down >= i-neighbourhood) and (down-1>=0) and (valid? down-1) and 
				(cluster[down-1] == cluster[i])
			down-=1;
		end
		(down..(i-1)).to_a + ((i+1)..up).to_a
	end
	
end