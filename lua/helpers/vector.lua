Vector = {};

function Vector:new(x, y, z)
	o = {};
	o.x = x or 0;
	o.y = y or 0;
	o.z = z or 0;
	setmetatable(o, self);
	self.__index = self;
	return o;
end

function Vector:set(x, y, z)
	self.x = x;
	self.y = y;
	self.z = z;
end

function Vector:print()
	print("x: " .. self.x .. " - y: " .. self.y .. " - z: " .. self.z .. "\n");
end

function Vector:dot(a)
	return self.x * a.x + self.y * a.y + self.z * a.z;
end

function Vector:__add(a)
	return Vector:new(a.x + self.x, a.y + self.y, a.z + self.z);
end

function Vector:__sub(a)
	return Vector:new(self.x - a.x, self.y - a.y, self.z - a.z);
end

function Vector:__tostring()
	return "x: " .. self.x .. " - y: " .. self.y .. " - z: " .. self.z;
end

function Vector:__eq(a)
	if self.x == a.x and self.y == a.y and self.z == a.z then
		return true;
	end
	return false;
end

function Vector:cross(a)
	b = Vector:new();
	b.x = self.y * a.z - self.z * a.y;
	b.y = self.z * a.x - self.x * a.z;
	b.z = self.x * a.y - self.y * a.x;
	return b;
end

function Vector:normalize()
	length = self.x * self.x + self.y * self.y + self.z * self.z;
	length = math.sqrt(length);

	self.x = self.x * (1/length);
	self.y = self.y * (1/length);
	self.z = self.z * (1/length);
end

function Vector:duplicate()
	return Vector:new(self.x, self.y, self.z);
end

function Vector:scale(x)
	self.x = self.x * x;
	self.y = self.y * x;
	self.z = self.z * x;
end

function Vector:components()
	return self.x, self.y, self.z;
end
