class ICP
	def kill_outliers(laser_ref, laser_sens)
		
		dists = (0..laser_ref.nrays-1).map { |i| 
			if
		}
		max_bin = 0.5;
		bins = 100
		perc = 0.4
		safe_level = 5

		hist = GSL::Histogram.alloc(bins, 0, max_bin)
		dists.each do |d| hist.fill2 d unless d.nil? end
		int = hist.integrate.scale(1.0/dists.compact.size)
		# XXX sono 
		#puts "Histo:"

#		for i in 0..bins-1
#			$stdout.write " #{hist[i]} "
#		end

		for i in 0..bins-1
			if int[i] > perc
				superato = i
				break
			end
		end

		nkilled = 0
		dists.each_index do |i|
			next if corrs[i].nil?
			bin = (dists[i]/max_bin)*bins;
	#		puts "#{i} bin=#{bin} dist=#{dists[i]}"
			if bin > (superato+1) * safe_level
#				puts "kill #{i} (d=#{dists[i]})"
				corrs[i] = nil;
				correspondences[i] = nil;
				nkilled += 1
			end
		end
		
		puts "killed #{nkilled}/#{corrs.size}"
	end
end