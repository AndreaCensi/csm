#!/usr/bin/env ruby

require 'gsl'
require 'structures'

module MathUtils
	include Math
	include GSL

	class PointCorrespondence
		attr_accessor :p
		attr_accessor :q
		attr_accessor :C
	end
	DEBUG = true
	

	def general_minimization(corr)

		# First we put the problem in a quadratic+constraint form.
		bigM = Matrix.alloc(4, 4).set_zero
		   g = Matrix.alloc(4, 1).set_zero

		for c in corr
			bigM_k = Matrix[[1, 0,  c.p[0], -c.p[1]],
			                [0, 1,  c.p[1],  c.p[0]]]
			
			bigM = bigM + 2*bigM_k.trans * c.C * bigM_k;
			g = g - 2 * (c.q.trans.to_m(1,2) * c.C * bigM_k).trans;
		end
		
		# Partition M in 4 submatrixes:
		# M = [A B; B' D]
		mA = bigM.submatrix(0,0,2,2)
		mB = bigM.submatrix(0,2,2,2)
		mD = bigM.submatrix(2,2,2,2)
		
		mS = mD - mB.trans * mA.inv * mB;
		mSa = mS.inv * mS.det;

		# split g in two
		g1 = g.submatrix(0,0,2,1)
		g2 = g.submatrix(2,0,2,1)

		p2 = g1.trans*mA.inv*mB*4*mB.trans*mA.inv*g1 
		     - 2*g1.trans*mA.inv*mB*4*g2 + g2.trans*4*g2
		p1 = g1.trans*mA.inv*mB*4*mSa*mB.trans*mA.inv*g1 -
		     2*g1.trans*mA.inv*mB*4*mSa*g2 +g2.trans*4*mSa*g2
		p0 = g1.trans*mA.inv*mB*mSa*mSa*mB.trans*mA.inv*g1 -
		     2*g1.trans*mA.inv*mB*mSa*mSa*g2 + g2.trans*mSa*mSa*g2

		p_lambda = Poly[mS[0,0]*mS[1,1]-mS[1,0]*mS[0,1],  2*mS[0,0]+2*mS[1,1],  4]

		p7 = Poly.alloc(p0[0,0],p1[0,0],p2[0,0])
		ptot = p7 - p_lambda * p_lambda

		r = ptot.roots
		lambda = 0
		## Find greatest real root
		for i in 0..3
			root = r[i]
			next if root.im!=0 
			lambda = [lambda, root.real].max
		end

		mW = Matrix[[0,0,0,0],
		           [0,0,0,0],
		           [0,0,1,0],
		           [0,0,0,1]]
		x = -(bigM + 2 * lambda * mW).inv * g;
		theta = Math.atan2(x[3,0],x[2,0]);

		return Vector.alloc(x[0,0],x[1,0],theta).col
	end

	def test_gm2
		
		theta = deg2rad(4);
		t = Vector.alloc(0.3,-0.2).col;
		
		p = [[1,0],[0,1],[-1, 0],[2,1],[4,2]];
		alphas = [deg2rad(0), deg2rad(10), deg2rad(20), deg2rad(50),deg2rad(-20)]

		corr = Array.new
		noise = 0
		5.times do |i| 
			c = PointCorrespondence.new
			c.p = Vector.alloc(p[i]).col
			c.q =  rot(theta)* c.p+t + Vector.alloc(rand-0.5,rand-0.5)*noise;
			alpha = alphas[i]
			c.C = vers(alpha)*vers(alpha).trans
			corr[i] = c
		end
		x = general_minimization(corr)

			puts " t, theta= #{t}, #{theta}"
			puts " x = #{x}"

	end
	def MathUtils.test_gm

		theta = rand*PI/10
		t = Vector.alloc(rand, rand).col

		corr = Array.new
		noise = 0
		10.times do |i| 
			c = PointCorrespondence.new
			c.p = Vector.alloc(rand, rand).col
			c.q =  rot(theta)* c.p+t + Vector.alloc(rand-0.5,rand-0.5)*noise;
			alpha = rand
			c.C = vers(alpha)*vers(alpha).trans
			corr[i] = c
		end
		x = general_minimization(corr)
	
		puts " t, theta= #{t}, #{theta}"
		puts " x = #{x}"
		
	end
end

