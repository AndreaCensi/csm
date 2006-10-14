class ICP
	def compute_covariance(laser_ref, laser_sens, x_new)
		fJ = create_J_function(laser_ref, laser_sens)

		mx  = x_new
		my1 = Vector.alloc(laser_ref.readings).col
		my2 = Vector.alloc(laser_sens.readings).col

#		tot = fJ.call(x,y1,y2)
#		puts "tot= #{tot} , total_error = #{@total_error}"

	 eps_x =0.001; eps_th = deg2rad(0.00001)
		d0 = Vector[eps_x,0, 0].col;
		d1 = Vector[0,eps_x, 0].col;
		d2 = Vector[0,0,eps_th].col;
		
		dJ_dx = Proc.new { |x,y1,y2|
			dJ_dx0 = (fJ.call(x,y1,y2)-fJ.call(x-d0,y1,y2))/(eps_x)
			dJ_dx1 = (fJ.call(x,y1,y2)-fJ.call(x-d1,y1,y2))/(eps_x)
			dJ_dx2 = (fJ.call(x,y1,y2)-fJ.call(x-d2,y1,y2))/(eps_th)
			Vector.alloc(dJ_dx0,dJ_dx1,dJ_dx2).col
		}
		d2J_dx2 = Proc.new { |x,y1,y2|
			d2J_dx0 = (dJ_dx.call(x,y1,y2)-dJ_dx.call(x-d0,y1,y2))/(eps_x);
			d2J_dx1 = (dJ_dx.call(x,y1,y2)-dJ_dx.call(x-d1,y1,y2))/(eps_x);
			d2J_dx2 = (dJ_dx.call(x,y1,y2)-dJ_dx.call(x-d2,y1,y2))/(eps_th);
			m = Matrix.alloc(3,3)
			m.set_col(0, d2J_dx0)
			m.set_col(1, d2J_dx1)
			m.set_col(2, d2J_dx2)
			m
		}
		
		d2J_dxdi = Proc.new { |x,y1,y2,i|
			delta_i = Vector.alloc(laser_sens.nrays).col
			delta_i.set_basis(i)
			delta_i = delta_i*eps_x;
			d = (dJ_dx.call(x,y1,y2+delta_i)-
				dJ_dx.call(x,y1,y2))/(eps_x);
				
			puts "d=#{d}"
			d
		}
		
		d2J_dxdj = Proc.new { |x,y1,y2,j|
			delta_j = Vector.alloc(laser_ref.nrays).col
			delta_j.set_basis(j)
			delta_j = delta_j *eps_x;
		#	puts "delta_j=#{delta_j}"
			x = (dJ_dx.call(x,y1+delta_j,y2)-
				dJ_dx.call(x,y1,y2))/(eps_x);
				
			puts "x= #{x}"
			x
		}
	
		d2J_dxdy2 = Matrix.alloc(3, laser_sens.nrays)
		for i in 0..laser_sens.nrays-1
			d2J_dxdy2.set_column(i, d2J_dxdi.call(mx,my1,my2,i))
		end

		d2J_dxdy1 = Matrix.alloc(3, laser_ref.nrays)
		for j in 0..laser_sens.nrays-1
			d2J_dxdy1.set_column(j, d2J_dxdj.call(mx,my1,my2,j))
		end
		
		dJ_dxee=  dJ_dx.call(mx,my1,my2)
		puts "deriv= #{dJ_dxee}"

		d2 = d2J_dx2.call(mx,my1,my2)
		puts "d2J_dx2 =\n #{d2}"

		d2 = 0.5*(d2.trans + d2)
		puts "sane =\n #{d2}"

		puts "d2J_dxdy1 = #{d2J_dxdy1.trans}"
		puts "d2J_dxdy2 = #{d2J_dxdy2.trans}"
		
		dx_dy1 = - d2.inv * d2J_dxdy1
		dx_dy2 = - d2.inv * d2J_dxdy2
		
		return dx_dy1, dx_dy2
	end
	
	def create_J_function(laser_ref, laser_sens)
		Proc.new { |x,y1,y2|
			fJtot = 0;
			
			for i in 0..laser_sens.nrays-1
				next if not laser_sens.corr[i].valid
				j1 = laser_sens.corr[i].j1
				j2 = laser_sens.corr[i].j2
				p_i  = laser_sens.v(i) * y2[i]
				p_j1 = laser_ref.v(j1) * y1[j1]
				p_j2 = laser_ref.v(j2) * y1[j2]

				v_alpha = rot(PI/2) * (p_j1-p_j2)
				v_alpha = v_alpha / v_alpha.nrm2
				m = v_alpha*v_alpha.trans
				p_i_w = transform(p_i, x)
				fJtot +=	(p_i_w-p_j1).trans * m * (p_i_w-p_j1)
			end
			fJtot 
		}
	end
end