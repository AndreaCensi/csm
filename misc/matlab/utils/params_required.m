
function params_required(p, field)
	if not(isfield(p, field))
		error('icp:bad_paramater',sprintf('I need field %s.', field));
	end

