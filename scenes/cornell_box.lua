-- scenes/cornell_box.lua
-- Cornell box approximation using ART's current geometry primitives.
-- No materials/lights yet — just geometry with normal-based coloring.

-- Box walls as scaled/translated planes
local function wall(tx, ty, tz, rx, ry, rz, angle)
    return {
        type = "plane",
        transform = {
            translate = {tx, ty, tz},
            rotate = {angle = angle, axis = {rx, ry, rz}},
        },
    }
end

return {
    render = {
        width = 400,
        height = 400,
        output = "cornell_box.ppm",
        accelerator = "bvh",
    },

    camera = {
        position = {0, 0, 4},
        target = {0, 0, 0},
        up = {0, 1, 0},
    },

    scene = {
        -- Back wall (z = -2)
        wall(0, 0, -2, 0, 0, 0, 0),

        -- Floor (y = -2)
        wall(0, -2, 0, 1, 0, 0, -math.pi / 2),

        -- Ceiling (y = 2)
        wall(0, 2, 0, 1, 0, 0, math.pi / 2),

        -- Left wall (x = -2)
        wall(-2, 0, 0, 0, 1, 0, math.pi / 2),

        -- Right wall (x = 2)
        wall(2, 0, 0, 0, 1, 0, -math.pi / 2),

        -- Tall box
        {
            type = "box",
            transform = {
                translate = {-0.5, -1, -0.5},
                rotate = {angle = 0.3, axis = {0, 1, 0}},
                scale = {1, 2, 1},
            },
        },

        -- Short box
        {
            type = "box",
            transform = {
                translate = {0.7, -1.5, 0.5},
                rotate = {angle = -0.2, axis = {0, 1, 0}},
            },
        },
    },
}
