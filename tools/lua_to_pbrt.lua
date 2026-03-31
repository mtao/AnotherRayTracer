#!/usr/bin/env lua
--- lua_to_pbrt — Convert ART Lua scene tables to PBRT v3 format.
---
--- Can be used as a module:
---   local l2p = require("lua_to_pbrt")
---   local pbrt_source = l2p.to_pbrt(scene_table)
---   l2p.convert_file("input.lua", "output.pbrt")
---
--- Or as a CLI script:
---   lua lua_to_pbrt.lua input.lua [output.pbrt]
---
--- Shape mapping (ART -> PBRT):
---   sphere        -> Shape "sphere"
---   disk          -> Shape "disk"
---   cylinder      -> Shape "cylinder"
---   box           -> Shape "trianglemesh" (tessellated to 12 triangles)
---   plane         -> Shape "disk" (large radius=1000)
---   triangle      -> Shape "trianglemesh" (1 triangle)
---   mesh          -> Shape "trianglemesh"
---   group         -> AttributeBegin / AttributeEnd
---
--- Transform mapping (ART -> PBRT):
---   translate     -> Translate x y z
---   rotate        -> Rotate degrees x y z
---   scale         -> Scale x y z
---   matrix        -> ConcatTransform [16 values]

local M = {}

-- ════════════════════════════════════════════════════════════════════
-- Utilities
-- ════════════════════════════════════════════════════════════════════

local function fmt_num(v)
    if v == math.floor(v) and math.abs(v) < 1e15 then
        return string.format("%g", v)
    end
    return string.format("%.10g", v)
end

local function indent(depth)
    return string.rep("  ", depth)
end

-- ════════════════════════════════════════════════════════════════════
-- Transform emission
-- ════════════════════════════════════════════════════════════════════

--- Emit PBRT transform directives for a transform table.
--- Returns a list of lines (without trailing newline).
local function emit_transform(xf, depth)
    if not xf then return {} end
    local lines = {}
    local pfx = indent(depth)

    -- PBRT applies transforms in reverse order of specification,
    -- so we emit in T, R, S order (same as ART's composition: T * R * S).
    if xf.translate then
        local t = xf.translate
        lines[#lines + 1] = pfx .. string.format("Translate %s %s %s",
            fmt_num(t[1]), fmt_num(t[2]), fmt_num(t[3]))
    end

    if xf.rotate then
        local r = xf.rotate
        local angle_deg = math.deg(r.angle)
        local a = r.axis
        lines[#lines + 1] = pfx .. string.format("Rotate %s %s %s %s",
            fmt_num(angle_deg), fmt_num(a[1]), fmt_num(a[2]), fmt_num(a[3]))
    end

    if xf.scale then
        if type(xf.scale) == "number" then
            local s = xf.scale
            lines[#lines + 1] = pfx .. string.format("Scale %s %s %s",
                fmt_num(s), fmt_num(s), fmt_num(s))
        else
            local s = xf.scale
            lines[#lines + 1] = pfx .. string.format("Scale %s %s %s",
                fmt_num(s[1]), fmt_num(s[2]), fmt_num(s[3]))
        end
    end

    if xf.matrix then
        local parts = {}
        for _, v in ipairs(xf.matrix) do
            parts[#parts + 1] = fmt_num(v)
        end
        lines[#lines + 1] = pfx .. "ConcatTransform [" .. table.concat(parts, " ") .. "]"
    end

    return lines
end

-- ════════════════════════════════════════════════════════════════════
-- Shape emission
-- ════════════════════════════════════════════════════════════════════

--- Emit a flat number array as a PBRT bracket list.
local function bracket_array(values)
    local parts = {}
    for _, v in ipairs(values) do
        parts[#parts + 1] = fmt_num(v)
    end
    return "[ " .. table.concat(parts, " ") .. " ]"
end

--- Emit a flat integer array as a PBRT bracket list.
local function bracket_int_array(values)
    local parts = {}
    for _, v in ipairs(values) do
        parts[#parts + 1] = string.format("%d", v)
    end
    return "[ " .. table.concat(parts, " ") .. " ]"
end

--- Box vertices and triangle indices (unit cube [-1,1]^3).
local box_verts = {
    {-1,-1,-1}, {1,-1,-1}, {1,1,-1}, {-1,1,-1},
    {-1,-1, 1}, {1,-1, 1}, {1,1, 1}, {-1,1, 1},
}
local box_tris = {
    {0,1,2}, {0,2,3},   -- front (-Z)
    {4,6,5}, {4,7,6},   -- back (+Z)
    {0,4,5}, {0,5,1},   -- bottom (-Y)
    {2,6,7}, {2,7,3},   -- top (+Y)
    {0,3,7}, {0,7,4},   -- left (-X)
    {1,5,6}, {1,6,2},   -- right (+X)
}

local function emit_node(node, depth, lines)
    if not node or not node.type then return end

    local pfx = indent(depth)

    -- Emit _comment as PBRT comment
    if node._comment then
        lines[#lines + 1] = pfx .. "# " .. node._comment
    end

    if node.type == "group" then
        lines[#lines + 1] = pfx .. "AttributeBegin"
        for _, line in ipairs(emit_transform(node.transform, depth + 1)) do
            lines[#lines + 1] = line
        end
        if node.children then
            for _, child in ipairs(node.children) do
                emit_node(child, depth + 1, lines)
            end
        end
        lines[#lines + 1] = pfx .. "AttributeEnd"

    elseif node.type == "sphere" then
        for _, line in ipairs(emit_transform(node.transform, depth)) do
            lines[#lines + 1] = line
        end
        lines[#lines + 1] = pfx .. 'Shape "sphere"'

    elseif node.type == "disk" then
        for _, line in ipairs(emit_transform(node.transform, depth)) do
            lines[#lines + 1] = line
        end
        lines[#lines + 1] = pfx .. 'Shape "disk"'

    elseif node.type == "cylinder" then
        for _, line in ipairs(emit_transform(node.transform, depth)) do
            lines[#lines + 1] = line
        end
        lines[#lines + 1] = pfx .. 'Shape "cylinder"'

    elseif node.type == "box" then
        -- Tessellate to trianglemesh
        for _, line in ipairs(emit_transform(node.transform, depth)) do
            lines[#lines + 1] = line
        end
        local flat_P = {}
        for _, v in ipairs(box_verts) do
            flat_P[#flat_P + 1] = v[1]
            flat_P[#flat_P + 1] = v[2]
            flat_P[#flat_P + 1] = v[3]
        end
        local flat_idx = {}
        for _, tri in ipairs(box_tris) do
            flat_idx[#flat_idx + 1] = tri[1]
            flat_idx[#flat_idx + 1] = tri[2]
            flat_idx[#flat_idx + 1] = tri[3]
        end
        lines[#lines + 1] = pfx .. 'Shape "trianglemesh"'
        lines[#lines + 1] = pfx .. '  "integer indices" ' .. bracket_int_array(flat_idx)
        lines[#lines + 1] = pfx .. '  "point3 P" ' .. bracket_array(flat_P)

    elseif node.type == "plane" then
        -- Emit as large disk
        for _, line in ipairs(emit_transform(node.transform, depth)) do
            lines[#lines + 1] = line
        end
        lines[#lines + 1] = pfx .. 'Shape "disk" "float radius" [1000]'

    elseif node.type == "triangle" then
        for _, line in ipairs(emit_transform(node.transform, depth)) do
            lines[#lines + 1] = line
        end
        if node.vertices and #node.vertices >= 3 then
            local flat_P = {}
            for i = 1, 3 do
                local v = node.vertices[i]
                flat_P[#flat_P + 1] = v[1]
                flat_P[#flat_P + 1] = v[2]
                flat_P[#flat_P + 1] = v[3]
            end
            lines[#lines + 1] = pfx .. 'Shape "trianglemesh"'
            lines[#lines + 1] = pfx .. '  "integer indices" [ 0 1 2 ]'
            lines[#lines + 1] = pfx .. '  "point3 P" ' .. bracket_array(flat_P)
        else
            lines[#lines + 1] = pfx .. '# triangle node missing vertices'
        end

    elseif node.type == "mesh" then
        for _, line in ipairs(emit_transform(node.transform, depth)) do
            lines[#lines + 1] = line
        end
        if node.vertices and node.triangles then
            local flat_P = {}
            for _, v in ipairs(node.vertices) do
                flat_P[#flat_P + 1] = v[1]
                flat_P[#flat_P + 1] = v[2]
                flat_P[#flat_P + 1] = v[3]
            end
            local flat_idx = {}
            for _, tri in ipairs(node.triangles) do
                flat_idx[#flat_idx + 1] = tri[1]
                flat_idx[#flat_idx + 1] = tri[2]
                flat_idx[#flat_idx + 1] = tri[3]
            end
            lines[#lines + 1] = pfx .. 'Shape "trianglemesh"'
            lines[#lines + 1] = pfx .. '  "integer indices" ' .. bracket_int_array(flat_idx)
            lines[#lines + 1] = pfx .. '  "point3 P" ' .. bracket_array(flat_P)
        else
            lines[#lines + 1] = pfx .. '# mesh node missing vertices or triangles'
        end

    else
        lines[#lines + 1] = pfx .. string.format('# unsupported ART type "%s"', node.type)
    end
end

-- ════════════════════════════════════════════════════════════════════
-- Module API
-- ════════════════════════════════════════════════════════════════════

--- Convert a scene table (ART Lua format) to PBRT source string.
--- The scene_table should have the same structure as what parse_file() returns,
--- or as what an ART Lua scene script returns (with render, camera, scene keys).
--- @param scene_table table   Table with .render, .camera, .scene keys
---                            (or a parse_file() result with .film_*, .camera_*, .scene)
--- @return string             PBRT source
function M.to_pbrt(scene_table)
    local lines = {}
    lines[#lines + 1] = "# Converted from ART Lua scene by lua_to_pbrt.lua"
    lines[#lines + 1] = ""

    -- Extract fields — support both direct scene tables and parse_file() results
    local render = scene_table.render or {}
    local width = render.width or scene_table.film_width or 800
    local height = render.height or scene_table.film_height or 600
    local filename = render.output or scene_table.film_filename or "output.ppm"

    local camera = scene_table.camera or {}
    local eye = camera.position or scene_table.camera_eye or {0, 0, 5}
    local target = camera.target or scene_table.camera_target or {0, 0, 0}
    local up = camera.up or scene_table.camera_up or {0, 1, 0}
    local fov = camera.fov or scene_table.camera_fov or 45

    local scene_nodes = scene_table.scene or {}

    -- Emit TODO comments if present
    if scene_table.todos and #scene_table.todos > 0 then
        local seen = {}
        for _, msg in ipairs(scene_table.todos) do
            if not seen[msg] then
                seen[msg] = true
                lines[#lines + 1] = "# TODO: " .. msg
            end
        end
        lines[#lines + 1] = ""
    end

    -- LookAt
    lines[#lines + 1] = string.format("LookAt %s %s %s  %s %s %s  %s %s %s",
        fmt_num(eye[1]), fmt_num(eye[2]), fmt_num(eye[3]),
        fmt_num(target[1]), fmt_num(target[2]), fmt_num(target[3]),
        fmt_num(up[1]), fmt_num(up[2]), fmt_num(up[3]))

    -- Camera
    lines[#lines + 1] = string.format('Camera "perspective" "float fov" [%s]', fmt_num(fov))

    -- Film
    lines[#lines + 1] = string.format(
        'Film "image" "integer xresolution" [%d] "integer yresolution" [%d] "string filename" ["%s"]',
        width, height, filename)

    lines[#lines + 1] = ""
    lines[#lines + 1] = "WorldBegin"

    -- Scene nodes
    for _, node in ipairs(scene_nodes) do
        emit_node(node, 1, lines)
    end

    lines[#lines + 1] = "WorldEnd"
    return table.concat(lines, "\n") .. "\n"
end

--- Load an ART Lua scene file and convert to PBRT.
--- @param input_path  string   Path to input .lua file
--- @param output_path string?  Path to output .pbrt file (nil = return string)
--- @return string|nil          PBRT source if output_path is nil
function M.convert_file(input_path, output_path)
    -- Load and execute the Lua scene file to get the scene table
    local chunk, err = loadfile(input_path)
    if not chunk then
        error(string.format("cannot load '%s': %s", input_path, err))
    end
    local ok, scene_table = pcall(chunk)
    if not ok then
        error(string.format("error executing '%s': %s", input_path, scene_table))
    end
    if type(scene_table) ~= "table" then
        error(string.format("'%s' did not return a table", input_path))
    end

    local pbrt_output = M.to_pbrt(scene_table)

    if output_path then
        local out, werr = io.open(output_path, "w")
        if not out then
            error(string.format("cannot open '%s' for writing: %s", output_path, werr or "unknown"))
        end
        out:write(pbrt_output)
        out:close()
        io.stderr:write(string.format("Converted %s -> %s\n", input_path, output_path))
        return nil
    end

    return pbrt_output
end

-- ════════════════════════════════════════════════════════════════════
-- CLI entry point
-- ════════════════════════════════════════════════════════════════════

local function main()
    if #arg < 1 then
        io.stderr:write("Usage: lua lua_to_pbrt.lua <input.lua> [output.pbrt]\n")
        os.exit(1)
    end

    local input_path = arg[1]
    local output_path = arg[2]

    if output_path then
        M.convert_file(input_path, output_path)
    else
        local pbrt_output = M.convert_file(input_path)
        io.write(pbrt_output)
    end
end

if arg and arg[0] and arg[0]:match("lua_to_pbrt%.lua$") then
    main()
end

return M
