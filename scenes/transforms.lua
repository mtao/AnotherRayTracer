-- scenes/transforms.lua
-- Demonstrates transform composition: multiple objects with
-- translation, rotation, and scale.

return {
    render = {
        width = 400,
        height = 300,
        output = "transforms.ppm",
        accelerator = "bvh",
    },

    camera = {
        position = {0, 2, 8},
        target = {0, 0, 0},
        up = {0, 1, 0},
    },

    scene = {
        -- Ground plane
        {
            type = "plane",
            transform = {
                translate = {0, -1, 0},
                rotate = {angle = -math.pi / 2, axis = {1, 0, 0}},
            },
        },

        -- Sphere on the left
        {
            type = "sphere",
            transform = {
                translate = {-2, 0, 0},
                scale = 0.5,
            },
        },

        -- Rotated box in the center
        {
            type = "box",
            transform = {
                translate = {0, 0, 0},
                rotate = {angle = 0.8, axis = {0, 1, 0}},
            },
        },

        -- Cylinder on the right
        {
            type = "cylinder",
            transform = {
                translate = {2, -1, 0},
                scale = {0.5, 2, 0.5},
            },
        },

        -- Disk floating above
        {
            type = "disk",
            transform = {
                translate = {0, 2, 0},
                scale = 1.5,
            },
        },
    },
}
