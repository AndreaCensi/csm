function res = params_set_default(p, field, default_value)
	% Checks whether the field is contained in p; if not, it adds the default_value.
	if not(isfield(p, field))
		p = setfield(p, field, default_value);
	%	fprintf('Setting default for %s = ', field);
	%	fprintf('%f ', default_value);
	%	fprintf('\n');
	end
	res = p;

