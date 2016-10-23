local module = {}

function module:setup()
   
end

return function(ctx)

    -- bind metatable
    setmetatable(ctx, { __index = module })

    ctx:setup()

    return ctx
end

