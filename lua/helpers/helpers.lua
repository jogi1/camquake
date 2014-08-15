module(..., package.seeall);

function compare(new, old, diff)
	if new == old then
		return old, diff;
	else
		return new, 1;
	end
end


