integer <- 5
local localinteger = 10

function foo(n)
{
    return n * integer
}

function vecsum(x, y)
{
    local v = Vec2(x, y)
    return v.sum()
}

function vecsum_withget(x, y)
{
    local v = getVec(x, y)
    return v.sum()
}