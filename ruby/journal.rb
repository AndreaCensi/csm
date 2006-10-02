
module Journal
	def initialized
		@open = false
	end
	
	def journal_laser(which, ld)
		return if not @open
		journal "laser #{which} nrays #{ld.nrays}" 
		journal "laser #{which} min_theta #{ld.min_theta}"
		journal "laser #{which} max_theta #{ld.max_theta}"
		journal "laser #{which} readings " +
		 	ld.points.map{ |p| p.reading }.join(" ")
	end 

	def journal_comment(line)
		return if not @open
		journal "# "+ line
	end
	
	def journal(line)
		return if not @open
		@file.puts line
	end
	
	def journal_open(fileName)
		@file = File.open( fileName, "w" )
		@open = true
		#$stderr.puts "Opened journal #{fileName}."
	end
	
	def journal_correspondences(s, corrs)
		return if not @open
		journal "#{s} " + 
			corrs.map{ |c| c.nil? ? '-1' : c.j1}.join(" ")
	end
	
	def to_j(x)
		return "undefined" if x.nil? 
		
		if x.kind_of? GSL::Matrix
			out = ""
			x.each_row do |r|
				r.each do |e|
					out << e << " "
				end
				out << "    "
			end
		end

		"#{x[0]} #{x[1]} #{x[2]}"
	end
end
