function interface(IN,OUT)
	local FloatPair = { a = Type:Float(), b = Type:Float() }
	IN.array = Type:Array(3, FloatPair)
	OUT.array = Type:Array(3, FloatPair)
end

function run(IN,OUT)
    OUT.array = IN.array
end
