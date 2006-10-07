class ICP
	def compute_covariance_exact(laser_ref, laser_sens, correspondences, x)
		y1 = Vector.alloc(laser_ref.points.map{|p| p.reading}).col
		y2 = Vector.alloc(laser_sens.points.map{|p| p.reading}).col

		d2J_dxdy2 = Matrix.alloc(3, laser_sens.nrays)
		d2J_dxdy1 = Matrix.alloc(3, laser_ref.nrays)
		
		t = Vector.alloc(x[0],x[1]).col
		theta = x[2].to_f;
		
		# the three pieces of d2J_dx2
		d2J_dt2       = Matrix.alloc(2,2)
		d2J_dt_dtheta = Vector.alloc(0,0).col
		d2J_dtheta2   = 0
		
		for c in correspondences.compact
			p_k  = laser_sens.points[c.i ].cartesian
			q_k  = laser_ref .points[c.j1].cartesian

			other  = laser_ref .points[c.j2].cartesian
			v_alpha = rot(PI/2) * (q_k-other)
			v_alpha = v_alpha / v_alpha.nrm2
			m = v_alpha*v_alpha.trans
			
			
			d2J_dt2       += 2 * m
			d2J_dt_dtheta += 2 * (rot(theta+PI/2)*p_k).trans * m
			d2J_dtheta2   += 2 * (rot(theta)*p_k+t-q_k).trans * 
			   m * rot(theta+PI/2) * p_k + 2 * (rot(theta+PI/2)*p_k).trans * m *
				rot(theta+PI/2) * p_k
				
			###########
			
				
			# for measurement rho_i  in the second scan
			v_i = laser_sens.points[c.i].v
			d2Jk_dtdrho_i = 2 * (rot(theta)*v_i).trans * m
			d2Jk_dtheta_drho_i = 2*(rot(theta)*p_k+t-q_k).trans*m*rot(theta+PI/2)*v_i +
				2 *(rot(theta)*v_i).trans*m*rot(theta+PI/2)*p_k
			
			d2J_dxdy2.col(c.i)[0] += d2Jk_dtdrho_i[0]
			d2J_dxdy2.col(c.i)[1] += d2Jk_dtdrho_i[1]
			d2J_dxdy2.col(c.i)[2] += d2Jk_dtheta_drho_i
		
			# for measurements rho_j1, rho_j2 in the first scan
			dC_drho_j1, dC_drho_j2 = dC_drho_j12(laser_ref, laser_sens, c)
			
			v_j1 = laser_ref.points[c.j1].v
			v_j2 = laser_ref.points[c.j2].v
			
			d2Jk_dtheta_drho_j1 = 2 * ( -v_j1.trans*m+(rot(theta)*p_k+t-q_k).trans*dC_drho_j1)*
				rot(theta+PI/2)*p_k
			d2Jk_dt_drho_j1 = 2 * (-v_j1.trans*m+(rot(theta)*p_k+t-q_k).trans*dC_drho_j1)
			
			d2J_dxdy1.col(c.j1)[0] += d2Jk_dt_drho_j1[0]
			d2J_dxdy1.col(c.j1)[1] += d2Jk_dt_drho_j1[1]
			d2J_dxdy1.col(c.j1)[2] += d2Jk_dtheta_drho_j1
			
			# for measurement rho_j2
			d2Jk_dtheta_drho_j2 = 2*(rot(theta)*p_k+t-q_k).trans * dC_drho_j2 *
				rot(theta+PI/2)*p_k;
				
			d2Jk_dt_drho_j2 = 2*(rot(theta)*p_k+t-q_k).trans * dC_drho_j2 
			
			d2J_dxdy1.col(c.j2)[0] += d2Jk_dt_drho_j2[0]
			d2J_dxdy1.col(c.j2)[1] += d2Jk_dt_drho_j2[1]
			d2J_dxdy1.col(c.j2)[2] += d2Jk_dtheta_drho_j2
			
		end
		# put the pieces together
		d2J_dx2 = Matrix.alloc(3,3)
		d2J_dx2[0,0]=d2J_dt2[0,0]
		d2J_dx2[1,0]=d2J_dt2[1,0]
		d2J_dx2[1,1]=d2J_dt2[1,1]
		d2J_dx2[0,1]=d2J_dt2[0,1]
		d2J_dx2[2,0]=d2J_dx2[0,2]=d2J_dt_dtheta[0]
		d2J_dx2[2,1]=d2J_dx2[1,2]=d2J_dt_dtheta[1]
		d2J_dx2[2,2] = d2J_dtheta2

		dx_dy1 =  -d2J_dx2.inv * d2J_dxdy1
		dx_dy2 =  -d2J_dx2.inv * d2J_dxdy2
		
		return dx_dy1, dx_dy2
	end
	
	def getC(rho_j1,v_j1,rho_j2,v_j2)
		p_j1 = v_j1 * rho_j1
		p_j2 = v_j2 * rho_j2
		v_alpha = rot(PI/2) * (p_j1-p_j2)
		v_alpha = v_alpha / v_alpha.nrm2
		m = v_alpha*(v_alpha.trans)
		m
	end
	
	def dC_drho_j12(laser_ref, laser_sens, c)
		
		rho_j1 = laser_ref.points[c.j1].reading
		  v_j1 = laser_ref.points[c.j1].v
		rho_j2 = laser_ref.points[c.j2].reading
		  v_j2 = laser_ref.points[c.j2].v
	
		eps = 0.001;
		
		dC_drho_j1 = 
		  (getC(rho_j1+eps,v_j1,rho_j2,v_j2)-
			getC(rho_j1    ,v_j1,rho_j2,v_j2))/eps;

		dC_drho_j2 = 
		  (getC(rho_j1,v_j1,rho_j2+eps,v_j2)-
			getC(rho_j1,v_j1,rho_j2    ,v_j2))/eps;
	
		return dC_drho_j1, dC_drho_j2
		
	end
end