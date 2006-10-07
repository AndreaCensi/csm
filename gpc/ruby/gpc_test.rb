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

require 'gpc'

module GPC
	def rot(a)
		GSL::Matrix[[Math.cos(a), -Math.sin(a)], [Math.sin(a), Math.cos(a)]]
	end
	
	def vers(a) 
		GSL::Vector.alloc(Math.cos(a), Math.sin(a)).col
	end
	
	def deg2rad(d)  d * Math::PI / 180  end
	def rad2deg(d)  d / Math::PI * 180  end
	
	def test
		theta = deg2rad(4);
		t = Vector.alloc(0.3,-0.2).col;
		
		p = [[1,0],[0,1],[-1, 0],[2,1],[4,2]];
		alphas = [deg2rad(0), deg2rad(10), deg2rad(20), deg2rad(50),deg2rad(-20)]

		corr = Array.new
		5.times do |i| 
			c = PointCorrespondence.new
			c.p = Vector.alloc(p[i]).col
			c.q =  rot(theta)* c.p+t;
			alpha = alphas[i]
			c.C = vers(alpha)*vers(alpha).trans
#			c.C = Matrix.eye(2)
			corr[i] = c
		end
		x = gpc(corr)

		puts "True: t (#{t[0]},#{t[1]}), theta = #{rad2deg(theta)}deg"
		puts "Estimated: t = #{x[0]} #{x[1]}  theta = #{rad2deg(x[2])}deg"
	end
	
	
	def test_random(noise)

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
		x = gpc(corr)
	
		puts "True: t (#{t[0]},#{t[1]}), theta = #{rad2deg(theta)}deg"
		puts "Estimated: t = #{x[0]} #{x[1]}  theta = #{rad2deg(x[2])}deg"
		
	end
	
end

include GPC
GPC::test
GPC::test_random(0)
GPC::test_random(2)


