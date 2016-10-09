--
-- Created by IntelliJ IDEA.
-- User: yayanyang
-- Date: 2016/10/9
-- Time: 上午10:56
-- To change this template use File | Settings | File Templates.
--

co = coroutine.create(function ()
    print("hi")
end)

print(coroutine.status(co))

coroutine.resume(co)

print("hello------------")
