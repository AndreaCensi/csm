#!/usr/bin/env ruby

# GPC: A library for the solution of General Point Correspondence problems.
# Copyright (C) 2006 Andrea Censi (andrea at censi dot org)
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

require 'gsl'

module GPC
	include Math
	include GSL

	class PointCorrespondence
		attr_accessor :p
		attr_accessor :q
		attr_accessor :C
	end
	
	DEBUG = false

	def GPC.gpc(corr)

		# First we put the problem in a quadratic+constraint form.
		bigM = Matrix.alloc(4, 4).set_zero
		   g = Matrix.alloc(4, 1).set_zero

		for c in corr
			bigM_k = Matrix[[1, 0,  c.p[0], -c.p[1]],
			                [0, 1,  c.p[1],  c.p[0]]]
			
			bigM = bigM + 2*bigM_k.trans * c.C * bigM_k;
			g = g - 2 * (c.q.trans.to_m(1,2) * c.C * bigM_k).trans;
			if DEBUG
				puts "M_k=\n#{bigM_k}\n"
				puts "q_k=\n#{c.q}\n"
				puts "C_k=\n#{c.C}\n"
				puts "now M is \n#{bigM}\n"
				puts "now g is \n#{g}\n"
			end
		end
		
		if DEBUG
			puts "M=#{bigM}"
		end
		
		# Partition M in 4 submatrixes:
		# M = [A B; B' D]
		mA = bigM.submatrix(0,0,2,2)
		mB = bigM.submatrix(0,2,2,2)
		mD = bigM.submatrix(2,2,2,2)
		
		mS = mD - mB.trans * mA.inv * mB;
		mSa = mS.inv * mS.det;

		if DEBUG
			puts "mA=#{mA}"
			puts "mB=#{mB}"
			puts "mD=#{mD}"
			puts "mS=#{mS}"
			puts "mSa=#{mSa}"
		end

		# split g in two
		g1 = g.submatrix(0,0,2,1)
		g2 = g.submatrix(2,0,2,1)

		p2 = g1.trans*mA.inv*mB*4*mB.trans*mA.inv*g1 -
		     2*g1.trans*mA.inv*mB*4*g2 + g2.trans*4*g2
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

		if DEBUG
			puts "p = [#{p2} #{p1} #{p0}]"
			puts "p_l = [#{p_lambda[2]} #{p_lambda[1]} #{p_lambda[0]}]"
			q = p_lambda * p_lambda;
			puts "p_l*p_l = #{q[4]}*x^4 +#{q[3]}*x^3 +#{q[2]}*x^2 +#{q[1]}*x +#{q[0]} "
			puts "ptot = #{ptot}"
			puts "roots = #{r[0]} #{r[1]} "
			puts "lambda = #{lambda}"
		end

		mW = Matrix[[0,0,0,0],
		           [0,0,0,0],
		           [0,0,1,0],
		           [0,0,0,1]]
		x = -(bigM + 2 * lambda * mW).inv * g;
		theta = Math.atan2(x[3,0],x[2,0]);

		return Vector.alloc(x[0,0],x[1,0],theta).col
	end

end

