-- scenes/sphere.lua
-- Simple sphere scene for ART Lua DSL.

return {
    render = {
        width = 200,
        height = 200,
        output = "sphere.ppm",
        accelerator = "bvh",
    },

    camera = {
        position = {0, 0, 5},
        target = {0, 0, 0},
        up = {0, 1, 0},
    },

    scene = {
        { type = "sphere" },
    },
}
