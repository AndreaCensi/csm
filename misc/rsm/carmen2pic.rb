#!/usr/bin/env ruby
require 'rsm'
require 'logreader'
require 'stringio'

class LaserData
	def valid_cartesian_points
		(0..nrays-1).map{|i|
			valid[i] ? p[i] : nil
		}.compact
	end
end

module MathUtils
	def transform_vector(v, x)
		v.map{|p| transform(p,x) }
	end
	
	def compute_bbox(points)
		points = points.compact
		bmin = Vector[points[0][0],points[0][1]].col
		bmax = Vector[points[0][0],points[0][1]].col
		points.each {|p|
			bmin[0] = min(bmin[0], p[0])
			bmin[1] = min(bmin[1], p[1])
			bmax[0] = max(bmax[0], p[0])
			bmax[1] = max(bmax[1], p[1])
		}
		return bmin, bmax
	end
end

require 'cairo'

class Canvas 
	def initialize(output, bbox_min, bbox_max)
		@bmin = bbox_min
		@bmax = bbox_max

		format = Cairo::FORMAT_ARGB32
		@width = 500
		@height = @width *( @bmax[1]-@bmin[1])/( @bmax[0]-@bmin[0])
		@output = StringIO.new
		surface = Cairo::PDFSurface.new(output, @width, @height)
		@cr = Cairo::Context.new(surface)

		# fill background with white
		@cr.set_source_rgba(1.0, 1.0, 1.0, 0.8)
		@cr.paint

		@cr.set_line_join(Cairo::LINE_JOIN_MITER)
		@cr.set_line_width 1
		
		@colors = [ 
			[1,0,0],[0.8,0,0],[0.7,0,0],[0.6,0,0],[0.5,0,0],
			[0,1,0],[0,0.8,0],[0,0.7,0],[0,0.6,0],[0,0.5,0],
		]
		@current_color = 0
		next_color
	end
	
	def save
		@cr.show_page

	 	@cr.target.finish  
#		@cr.target.write_to_png(file)		
	end
	
	def w2b(p)
		x = (p[0] - @bmin[0]) / (@bmax[0]-@bmin[0]) * @width
		y = (p[1] - @bmin[1]) / (@bmax[1]-@bmin[1]) * @height
		[x, y]
	end
	
	def move_to(p)
		buf = w2b(p)
		@cr.move_to(buf[0],buf[1])
	end

	def line_to(p)
		buf = w2b(p)
		@cr.line_to(buf[0],buf[1])
	end
	
	def next_color
		@current_color = (@current_color+1) % @colors.size
		rgb = @colors[@current_color]
		@cr.set_source_rgb(rgb[0],rgb[1],rgb[2])
	end
	
	def write_points(points)
		return if points.empty?
		@cr.set_line_cap(Cairo::LINE_CAP_ROUND)
		@cr.set_line_width(1)
		next_color
		move_to(w2b(points[0]))
		points.each do |p|
			b = w2b(p)
#			@cr.arc( b[0], b[1], 1, 0, 2*M_PI);
#			@cr.fill
			@cr.move_to(b[0],b[1])
			@cr.close_path
			@cr.stroke
		end
	end
	def cr; @cr end
end

def carmen2pic(input,output_file)
	include MathUtils
	
	count = 0; interval = 10
	lds = []
	until input.eof? 
		ld = LogReader.shift_laser(input)
		
		break if ld.nil? 
		
		if count%interval == 0
			puts count
			lds.push ld
		end
		count+=1
	end
	
	puts "Read #{lds.size} laser scans."
	
	bbox_min = nil
	bbox_max = nil
	all_points = []
	
	lds.each_index do |i|
		ld = lds[i]
		ld.compute_cartesian
		points = ld.valid_cartesian_points
		points = transform_vector(points, ld.estimate)
		bbox_min, bbox_max = compute_bbox(points+[bbox_min,bbox_max]) 
		all_points << points
	end
	puts "bbox: #{bbox_min.row} to #{bbox_max}"
	
	output = File.open(output_file ,"wb")
	ca = Canvas.new(output, bbox_min, bbox_max)

	ca.cr.set_line_width(2)
	ca.move_to(lds[0].estimate)
		lds.each_index do |i|
			ca.line_to(lds[i].estimate)
		end
	ca.cr.stroke

	lds.each_index do |i|
		ca.write_points(all_points[i])
	end
	
	ca.save()
end

if ARGV.size < 2
	puts "carmen2pic <input> <output> "
	exit 1
end


input = File.open ARGV[0]
output_file = ARGV[1]

carmen2pic(input,output_file)
