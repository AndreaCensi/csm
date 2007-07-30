function s = matlab_to_json(ob, pretty)
	s = '!';
	
	if isnumeric(ob)
		if (size(ob,1) == 1) && (size(ob,2) == 1) 
			if isnan(ob)
				s = 'null';
			else
				if isinteger(ob)
					s = sprintf('%d', ob);
				else
					s = sprintf('%f', ob);
				end
			end
		else
			% XXXXXXX non funziona se row=1 col>1
			% array
			s = '[';
			rows = size(ob, 1);
			for row=1:rows
				v = ob(row,:);
				s = strcat(s, matlab_to_json(v) );
				if row < rows
					s = strcat(s, ', ');
				end
				
				if (rows>1) && (size(ob,2) >1) && pretty
					s = strcat(s, '\n');
				end
				
			end
			s = strcat(s, ']');
		end
		return;
	end
	
	if isstruct(ob)
		s = '{ ';
		names = fieldnames(ob);
		for i=1:size(names,1)
			key = names{i};
			value = matlab_to_json( getfield(ob, key) );
			s = strcat(s, '"', key, '": ', value);
			if i < size(names, 1) 
				s = strcat(s, ', ');
				
				if pretty
					s = strcat(s, '\n');
				end
			end
		end
		s = strcat(s, '}');
		return;
	end
	
	s = '?';
	




