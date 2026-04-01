-- scenes/mesh_demo.lua
-- Demonstrates inline mesh construction via Lua tables.

return {
    render = {
        width = 200,
        height = 200,
        output = "mesh_demo.ppm",
        accelerator = "bvh",
    },

    camera = {
        position = {2, 2, 5},
        target = {0, 0, 0},
        up = {0, 1, 0},
    },

    scene = {
        -- A cube mesh
        {
            type = "cube_mesh",
            transform = {
                rotate = {angle = 0.5, axis = {1, 1, 0}},
            },
        },

        -- A custom triangle
        {
            type = "triangle",
            vertices = {
                {-2, -1, 0},
                {-1, -1, 0},
                {-1.5, 0, 0},
            },
        },
    },
}
