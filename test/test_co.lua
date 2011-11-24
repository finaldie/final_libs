

function test(num)
	print(num)

	print(coroutine.running())
	local n = coroutine.yield()
	print(coroutine.running())
	print(n)
end

co = coroutine.create(test)
print(co)
coroutine.resume(co, 1)
coroutine.resume(co, 2)
