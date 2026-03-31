#!/usr/bin/env lua
--- pbrt_to_lua — Convert PBRT v3/v4 scene files to ART Lua table DSL.
---
--- Can be used as a module:
---   local pbrt = require("pbrt_to_lua")
---   local scene, todos = pbrt.parse_file("input.pbrt")
---   local lua_source    = pbrt.to_lua(scene, todos)
---   pbrt.convert_file("input.pbrt", "output.lua")
---
--- Or as a CLI script:
---   lua pbrt_to_lua.lua input.pbrt [output.lua]
---
--- Supported PBRT directives:
---   LookAt, Camera ("perspective"), Film ("image" / "rgb")
---   WorldBegin / WorldEnd (v3; v4 ends at EOF)
---   AttributeBegin / AttributeEnd
---   TransformBegin / TransformEnd (v3)
---   Translate, Rotate, Scale, Identity, ConcatTransform, Transform
---   Shape ("sphere", "trianglemesh", "disk", "cylinder", "plymesh",
---          "bilinearmesh", "loopsubdiv", "curve", "cone", "paraboloid",
---          "hyperboloid")
---   ObjectBegin / ObjectEnd, ObjectInstance
---   Include, Import (v4)
---
--- Unsupported directives (Material, LightSource, Texture, Integrator,
--- Sampler, etc.) are consumed gracefully and emitted as TODO comments.

local M = {}

-- ════════════════════════════════════════════════════════════════════
-- Utilities
-- ════════════════════════════════════════════════════════════════════

local function deep_copy(orig)
    if type(orig) ~= "table" then return orig end
    local copy = {}
    for k, v in pairs(orig) do
        copy[k] = deep_copy(v)
    end
    return copy
end

-- ════════════════════════════════════════════════════════════════════
-- Tokenizer
-- ════════════════════════════════════════════════════════════════════

--- Tokenize a PBRT source string.
--- Handles: quoted strings, brackets, numbers (incl. .5 and 1e-3),
--- identifiers, and bare true/false (v4 bool params).
--- Comments (#) are stripped.
local function tokenize(source)
    local tokens = {}
    local pos = 1
    local len = #source

    while pos <= len do
        -- Skip whitespace
        local ws = source:match("^%s+", pos)
        if ws then pos = pos + #ws end
        if pos > len then break end

        local ch = source:sub(pos, pos)

        -- Comment: skip to end of line
        if ch == "#" then
            local nl = source:find("\n", pos)
            pos = nl and nl + 1 or len + 1

        -- Quoted string
        elseif ch == '"' then
            local close = source:find('"', pos + 1)
            if not close then
                io.stderr:write("warning: unterminated string at position " .. pos .. "\n")
                break
            end
            tokens[#tokens + 1] = { type = "string", value = source:sub(pos + 1, close - 1) }
            pos = close + 1

        -- Open/close bracket
        elseif ch == "[" then
            tokens[#tokens + 1] = { type = "open_bracket" }
            pos = pos + 1

        elseif ch == "]" then
            tokens[#tokens + 1] = { type = "close_bracket" }
            pos = pos + 1

        -- Number (including negative, decimal, scientific, leading-dot like .5)
        else
            local num = source:match("^%-?%d*%.?%d+[eE][%-+]?%d+", pos)
                      or source:match("^%-?%d*%.%d+", pos)
                      or source:match("^%-?%d+", pos)
            if num then
                tokens[#tokens + 1] = { type = "number", value = tonumber(num) }
                pos = pos + #num
            else
                -- Identifier / directive / bare true/false
                local id = source:match("^[A-Za-z_][A-Za-z0-9_]*", pos)
                if id then
                    -- v4 uses bare true/false in bracket arrays
                    if id == "true" then
                        tokens[#tokens + 1] = { type = "bool", value = true }
                    elseif id == "false" then
                        tokens[#tokens + 1] = { type = "bool", value = false }
                    else
                        tokens[#tokens + 1] = { type = "identifier", value = id }
                    end
                    pos = pos + #id
                else
                    -- Skip unknown character
                    pos = pos + 1
                end
            end
        end
    end

    return tokens
end

M.tokenize = tokenize

-- ════════════════════════════════════════════════════════════════════
-- Parser state
-- ════════════════════════════════════════════════════════════════════

local Parser = {}
Parser.__index = Parser

function Parser.new(tokens)
    return setmetatable({
        tokens = tokens,
        pos = 1,
        -- Camera
        camera_eye = nil,
        camera_target = nil,
        camera_up = nil,
        camera_fov = 45,
        camera_type = "perspective",
        -- Film
        film_width = 800,
        film_height = 600,
        film_filename = "output.ppm",
        -- Transform stack (each entry is a list of transform ops)
        transform_stack = {},
        current_transforms = {}, -- list of {type, ...} ops
        -- Scene nodes
        scene = {},               -- top-level scene array
        node_stack = {},          -- stack for group nesting
        -- Named objects
        objects = {},             -- name -> node array
        current_object_name = nil,
        -- Base directory for Include / Import
        base_dir = ".",
        -- Collected TODO comments for unsupported features
        todos = {},               -- list of strings
    }, Parser)
end

function Parser:peek()
    return self.tokens[self.pos]
end

function Parser:advance()
    local t = self.tokens[self.pos]
    self.pos = self.pos + 1
    return t
end

function Parser:expect_type(typ)
    local t = self:advance()
    if not t or t.type ~= typ then
        io.stderr:write(string.format(
            "warning: expected %s, got %s at token %d\n",
            typ, t and t.type or "EOF", self.pos - 1))
        return nil
    end
    return t
end

--- Read N numbers from the token stream.
function Parser:read_numbers(n)
    local nums = {}
    for i = 1, n do
        local t = self:advance()
        if t and t.type == "number" then
            nums[i] = t.value
        else
            io.stderr:write(string.format(
                "warning: expected number %d/%d, got %s\n",
                i, n, t and t.type or "EOF"))
            nums[i] = 0
        end
    end
    return nums
end

--- Read a bracketed array: [ n1 n2 ... ] or [ "string" ] or [ true/false ]
--- Returns a list of numbers, strings, or booleans.
function Parser:read_bracket_array()
    local t = self:peek()
    if not t or t.type ~= "open_bracket" then return nil end
    self:advance() -- consume [

    local values = {}
    while true do
        t = self:peek()
        if not t or t.type == "close_bracket" then break end
        if t.type == "number" then
            values[#values + 1] = t.value
            self:advance()
        elseif t.type == "string" then
            values[#values + 1] = t.value
            self:advance()
        elseif t.type == "bool" then
            values[#values + 1] = t.value
            self:advance()
        else
            break
        end
    end

    self:expect_type("close_bracket")
    return values
end

--- Read PBRT parameter list: "type name" [values] pairs.
--- Returns a table mapping param name -> { type = "float"|"integer"|"point3"|..., values = {...} }
function Parser:read_params()
    local params = {}
    while true do
        local t = self:peek()
        if not t then break end
        if t.type ~= "string" then break end

        local param_spec = t.value
        self:advance()

        -- Parse "type name" format (supports multi-word types like "point3", "normal3", "rgb")
        local ptype, pname = param_spec:match("^(%S+)%s+(%S+)$")
        if not ptype then
            -- Some PBRT files use just the name
            pname = param_spec
            ptype = "unknown"
        end

        -- Read the value: bracketed array, bare string, bare number, or bare bool
        local next_tok = self:peek()
        if next_tok and next_tok.type == "open_bracket" then
            params[pname] = { type = ptype, values = self:read_bracket_array() }
        elseif next_tok and next_tok.type == "string" then
            -- Check this isn't the start of the next param spec (has "type name" format)
            if next_tok.value:match("^%S+%s+%S+$") then
                -- It's another param spec, not a value — store empty
                params[pname] = { type = ptype, values = {} }
            else
                params[pname] = { type = ptype, values = { next_tok.value } }
                self:advance()
            end
        elseif next_tok and next_tok.type == "number" then
            params[pname] = { type = ptype, values = { next_tok.value } }
            self:advance()
        elseif next_tok and next_tok.type == "bool" then
            params[pname] = { type = ptype, values = { next_tok.value } }
            self:advance()
        else
            -- No value (flag-style param or missing value)
            params[pname] = { type = ptype, values = {} }
        end
    end
    return params
end

--- Get the current node list (top of stack or scene root).
function Parser:current_nodes()
    if self.current_object_name then
        return self.objects[self.current_object_name]
    end
    if #self.node_stack > 0 then
        return self.node_stack[#self.node_stack].children
    end
    return self.scene
end

--- Try to flatten a list of transform ops into a single {translate, rotate, scale}.
--- Returns a flat table if possible, nil otherwise.
--- Flattening works when we have at most one of each in T, R, S order.
local function try_flatten_transforms(ops)
    if #ops == 0 then return nil end

    local translate, rotate, scale, matrix
    local seen_types = {}

    for _, op in ipairs(ops) do
        if seen_types[op.type] then return nil end
        seen_types[op.type] = true

        if op.type == "translate" then
            if seen_types["rotate"] or seen_types["scale"] or seen_types["matrix"] then return nil end
            translate = op.value
        elseif op.type == "rotate" then
            if seen_types["scale"] or seen_types["matrix"] then return nil end
            rotate = op.value
        elseif op.type == "scale" then
            if seen_types["matrix"] then return nil end
            scale = op.value
        elseif op.type == "matrix" then
            matrix = op.value
        else
            return nil
        end
    end

    local t = {}
    local has_any = false
    if translate then t.translate = translate; has_any = true end
    if rotate then t.rotate = rotate; has_any = true end
    if scale then t.scale = scale; has_any = true end
    if matrix then t.matrix = matrix; has_any = true end
    return has_any and t or nil
end

--- Convert a single transform op to a flat TRS table.
local function op_to_transform(op)
    if op.type == "translate" then
        return { translate = op.value }
    elseif op.type == "rotate" then
        return { rotate = op.value }
    elseif op.type == "scale" then
        return { scale = op.value }
    elseif op.type == "matrix" then
        return { matrix = op.value }
    end
    return nil
end

--- Add a scene node to the current context.
--- If the accumulated transforms can be flattened into a single TRS, attach directly.
--- Otherwise, wrap in nested groups (one per transform op, outermost = first op).
function Parser:add_node(node)
    if #self.current_transforms == 0 then
        local nodes = self:current_nodes()
        nodes[#nodes + 1] = node
        return
    end

    local flat = try_flatten_transforms(self.current_transforms)
    if flat then
        node.transform = flat
        local nodes = self:current_nodes()
        nodes[#nodes + 1] = node
        return
    end

    -- Cannot flatten: wrap in nested groups.
    -- PBRT applies transforms right-to-left (last specified = applied first).
    -- Each group gets one transform, innermost = last op.
    -- Build inside-out: start with the leaf node + last op, wrap outward.
    local inner = node
    for i = #self.current_transforms, 1, -1 do
        local xf = op_to_transform(self.current_transforms[i])
        if xf then
            inner.transform = xf
            if i > 1 then
                inner = { type = "group", children = { inner } }
            end
        end
    end

    local nodes = self:current_nodes()
    nodes[#nodes + 1] = inner
end

--- Record a TODO for an unsupported PBRT feature.
function Parser:add_todo(msg)
    self.todos[#self.todos + 1] = msg
end

-- ════════════════════════════════════════════════════════════════════
-- Directive handlers
-- ════════════════════════════════════════════════════════════════════

function Parser:handle_LookAt()
    local nums = self:read_numbers(9)
    self.camera_eye = { nums[1], nums[2], nums[3] }
    self.camera_target = { nums[4], nums[5], nums[6] }
    self.camera_up = { nums[7], nums[8], nums[9] }
end

function Parser:handle_Camera()
    local type_tok = self:expect_type("string")
    local params = self:read_params()
    if params.fov and params.fov.values[1] then
        self.camera_fov = params.fov.values[1]
    end
    self.camera_type = type_tok and type_tok.value or "perspective"
    if self.camera_type ~= "perspective" then
        self:add_todo(string.format(
            "Camera type '%s' not supported by ART (only 'perspective')", self.camera_type))
    end
    -- Collect unsupported camera params
    for name, _ in pairs(params) do
        if name ~= "fov" then
            self:add_todo(string.format("Camera param '%s' not supported by ART", name))
        end
    end
end

function Parser:handle_Film()
    local type_tok = self:expect_type("string")
    local film_type = type_tok and type_tok.value or "image"
    local params = self:read_params()
    if params.xresolution and params.xresolution.values[1] then
        self.film_width = math.floor(params.xresolution.values[1])
    end
    if params.yresolution and params.yresolution.values[1] then
        self.film_height = math.floor(params.yresolution.values[1])
    end
    if params.filename and params.filename.values[1] then
        -- Convert extension to .ppm since ART outputs PPM by default
        local name = params.filename.values[1]
        name = name:gsub("%.[^.]+$", ".ppm")
        self.film_filename = name
    end
    -- Film type handling: "image" (v3), "rgb" (v4), "gbuffer", "spectral"
    if film_type ~= "image" and film_type ~= "rgb" then
        self:add_todo(string.format(
            "Film type '%s' not supported by ART (using default output)", film_type))
    end
end

function Parser:handle_WorldBegin()
    -- Reset transform for world scope
    self.current_transforms = {}
end

function Parser:handle_WorldEnd()
    -- Nothing to do (v3 only; v4 doesn't have this)
end

function Parser:handle_AttributeBegin()
    -- Push current transform state (deep copy)
    self.transform_stack[#self.transform_stack + 1] = deep_copy(self.current_transforms)
    -- Push a new group scope (children collected here)
    self.node_stack[#self.node_stack + 1] = {
        children = {},
        transforms = deep_copy(self.current_transforms),
    }
    -- Reset transform for children (relative to this scope)
    self.current_transforms = {}
end

function Parser:handle_AttributeEnd()
    -- Pop the group scope
    local scope = table.remove(self.node_stack)
    if not scope then
        io.stderr:write("warning: AttributeEnd without matching AttributeBegin\n")
        return
    end

    -- Restore previous transform
    self.current_transforms = table.remove(self.transform_stack) or {}

    -- If the scope has children, emit them
    if #scope.children == 1 then
        -- Single child: emit directly
        local child = scope.children[1]
        local parent_nodes = self:current_nodes()
        parent_nodes[#parent_nodes + 1] = child
    elseif #scope.children > 1 then
        -- Multiple children: wrap in a group
        local group = { type = "group", children = scope.children }
        local parent_nodes = self:current_nodes()
        parent_nodes[#parent_nodes + 1] = group
    end
end

function Parser:handle_TransformBegin()
    self.transform_stack[#self.transform_stack + 1] = deep_copy(self.current_transforms)
end

function Parser:handle_TransformEnd()
    self.current_transforms = table.remove(self.transform_stack) or {}
end

function Parser:handle_Translate()
    local nums = self:read_numbers(3)
    self.current_transforms[#self.current_transforms + 1] = {
        type = "translate",
        value = { nums[1], nums[2], nums[3] },
    }
end

function Parser:handle_Rotate()
    -- PBRT: Rotate angle x y z (angle in degrees)
    local nums = self:read_numbers(4)
    self.current_transforms[#self.current_transforms + 1] = {
        type = "rotate",
        value = {
            angle = math.rad(nums[1]),
            axis = { nums[2], nums[3], nums[4] },
        },
    }
end

function Parser:handle_Scale()
    local nums = self:read_numbers(3)
    local scale_val
    if nums[1] == nums[2] and nums[2] == nums[3] then
        scale_val = nums[1]
    else
        scale_val = { nums[1], nums[2], nums[3] }
    end
    self.current_transforms[#self.current_transforms + 1] = {
        type = "scale",
        value = scale_val,
    }
end

function Parser:handle_Identity()
    self.current_transforms = {}
end

function Parser:handle_ConcatTransform()
    local vals = self:read_bracket_array()
    if vals and #vals == 16 then
        -- Store the raw 4x4 matrix (column-major, as PBRT specifies).
        -- ART's parse_transform doesn't read 'matrix' yet, but we
        -- preserve it for future use / round-trip fidelity.
        self.current_transforms[#self.current_transforms + 1] = {
            type = "matrix",
            value = vals,
        }
    else
        io.stderr:write("warning: ConcatTransform expected 16 values\n")
        self:add_todo("ConcatTransform with unexpected element count")
    end
end

function Parser:handle_Transform()
    -- Transform replaces the CTM entirely (vs ConcatTransform which multiplies)
    -- For our purposes we treat it the same way: store the matrix.
    self.current_transforms = {} -- replace, not accumulate
    local vals = self:read_bracket_array()
    if vals and #vals == 16 then
        self.current_transforms[#self.current_transforms + 1] = {
            type = "matrix",
            value = vals,
        }
    else
        io.stderr:write("warning: Transform expected 16 values\n")
        self:add_todo("Transform with unexpected element count")
    end
end

function Parser:handle_Shape()
    local type_tok = self:expect_type("string")
    if not type_tok then return end
    local shape_type = type_tok.value
    local params = self:read_params()

    if shape_type == "sphere" then
        local node = { type = "sphere" }
        -- PBRT sphere has a radius param; ART sphere is unit, so scale
        if params.radius and params.radius.values[1] then
            local r = params.radius.values[1]
            if r ~= 1 then
                self.current_transforms[#self.current_transforms + 1] = {
                    type = "scale",
                    value = r,
                }
            end
        end
        self:add_node(node)

    elseif shape_type == "disk" then
        -- PBRT disk can have height, radius, innerradius, phimax
        local node = { type = "disk" }
        if params.radius and params.radius.values[1] and params.radius.values[1] ~= 1 then
            self.current_transforms[#self.current_transforms + 1] = {
                type = "scale",
                value = params.radius.values[1],
            }
        end
        if params.height and params.height.values[1] and params.height.values[1] ~= 0 then
            self.current_transforms[#self.current_transforms + 1] = {
                type = "translate",
                value = { 0, 0, params.height.values[1] },
            }
        end
        self:add_node(node)

    elseif shape_type == "cylinder" then
        local node = { type = "cylinder" }
        self:add_node(node)

    elseif shape_type == "cone" then
        -- PBRT cone: radius, height, phimax — no direct ART equivalent
        local node = {
            type = "sphere",
            _comment = string.format(
                "TODO: cone (radius=%s, height=%s) — no ART cone primitive",
                params.radius and tostring(params.radius.values[1]) or "1",
                params.height and tostring(params.height.values[1]) or "1"),
        }
        self:add_node(node)
        self:add_todo("Shape 'cone' emitted as placeholder sphere (no ART cone primitive)")

    elseif shape_type == "paraboloid" then
        local node = {
            type = "sphere",
            _comment = string.format(
                "TODO: paraboloid (radius=%s, zmin=%s, zmax=%s) — no ART paraboloid primitive",
                params.radius and tostring(params.radius.values[1]) or "1",
                params.zmin and tostring(params.zmin.values[1]) or "0",
                params.zmax and tostring(params.zmax.values[1]) or "1"),
        }
        self:add_node(node)
        self:add_todo("Shape 'paraboloid' emitted as placeholder sphere (no ART paraboloid primitive)")

    elseif shape_type == "hyperboloid" then
        local node = {
            type = "sphere",
            _comment = "TODO: hyperboloid — no ART hyperboloid primitive",
        }
        self:add_node(node)
        self:add_todo("Shape 'hyperboloid' emitted as placeholder sphere (no ART hyperboloid primitive)")

    elseif shape_type == "curve" then
        -- PBRT curve: extremely common (5.9M instances in test corpus).
        -- Parameters: type (flat/ribbon/cylinder), P (control points),
        -- width/width0/width1, splitdepth, etc.
        local curve_type = params.type and params.type.values[1] or "flat"
        local npts = params.P and #params.P.values / 3 or 0
        local node = {
            type = "sphere",
            _comment = string.format(
                "TODO: curve (type='%s', %d control points) — no ART curve primitive",
                curve_type, npts),
        }
        self:add_node(node)
        self:add_todo("Shape 'curve' emitted as placeholder sphere (no ART curve primitive)")

    elseif shape_type == "trianglemesh" then
        local indices = params.indices or params.index
        local points = params.P or params.point

        if indices and points then
            -- Convert flat arrays to structured data
            local verts = {}
            for i = 1, #points.values, 3 do
                verts[#verts + 1] = {
                    points.values[i],
                    points.values[i + 1],
                    points.values[i + 2],
                }
            end

            local tris = {}
            for i = 1, #indices.values, 3 do
                tris[#tris + 1] = {
                    indices.values[i],
                    indices.values[i + 1],
                    indices.values[i + 2],
                }
            end

            local node = {
                type = "mesh",
                vertices = verts,
                triangles = tris,
            }

            -- Extract normals if present
            local normals = params.N or params.normal
            if normals and #normals.values >= 3 then
                node._comment = "TODO: normals present but not emitted"
                self:add_todo("trianglemesh normals (N) not yet mapped to ART mesh")
            end

            -- Extract UVs if present
            local uvs = params.uv or params.st
            if uvs and #uvs.values >= 2 then
                if not node._comment then
                    node._comment = "TODO: UVs present but not emitted"
                end
                self:add_todo("trianglemesh UVs (uv/st) not yet mapped to ART mesh")
            end

            self:add_node(node)
        else
            -- Emit as comment
            self:add_node({
                type = "sphere", -- placeholder
                _comment = "TODO: trianglemesh missing indices or P data",
            })
        end

    elseif shape_type == "bilinearmesh" then
        -- v4: quad mesh — each quad has 4 vertex indices, tessellate into 2 triangles
        local indices = params.indices or params.index
        local points = params.P or params.point

        if indices and points then
            local verts = {}
            for i = 1, #points.values, 3 do
                verts[#verts + 1] = {
                    points.values[i],
                    points.values[i + 1],
                    points.values[i + 2],
                }
            end

            -- Quad indices -> triangle pairs
            local tris = {}
            for i = 1, #indices.values, 4 do
                local v0 = indices.values[i]
                local v1 = indices.values[i + 1]
                local v2 = indices.values[i + 2]
                local v3 = indices.values[i + 3]
                tris[#tris + 1] = { v0, v1, v2 }
                tris[#tris + 1] = { v0, v2, v3 }
            end

            local node = {
                type = "mesh",
                vertices = verts,
                triangles = tris,
                _comment = "bilinearmesh tessellated to triangles",
            }
            self:add_node(node)
        else
            self:add_node({
                type = "sphere",
                _comment = "TODO: bilinearmesh missing indices or P data",
            })
        end

    elseif shape_type == "loopsubdiv" then
        -- Loop subdivision surface — emit with mesh data if available
        local indices = params.indices or params.index
        local points = params.P or params.point

        if indices and points then
            local verts = {}
            for i = 1, #points.values, 3 do
                verts[#verts + 1] = {
                    points.values[i],
                    points.values[i + 1],
                    points.values[i + 2],
                }
            end

            local tris = {}
            for i = 1, #indices.values, 3 do
                tris[#tris + 1] = {
                    indices.values[i],
                    indices.values[i + 1],
                    indices.values[i + 2],
                }
            end

            self:add_node({
                type = "mesh",
                vertices = verts,
                triangles = tris,
                _comment = "TODO: loopsubdiv (control mesh only, no subdivision applied)",
            })
        else
            self:add_node({
                type = "sphere",
                _comment = "TODO: loopsubdiv without inline mesh data",
            })
        end
        self:add_todo("Loop subdivision surface emitted as control mesh (no subdivision applied)")

    elseif shape_type == "plymesh" then
        -- PLY mesh: emit as TODO comment with path
        -- TODO: add PLY reading to quiver (quiver::utils::io::PlyReader)
        local filename = params.filename and params.filename.values[1]
        self:add_node({
            type = "sphere", -- placeholder
            _comment = string.format("TODO: plymesh '%s' — load via quiver I/O (quiver PLY reader needed)",
                filename or "unknown"),
        })
        self:add_todo(string.format("PLY mesh '%s' requires quiver PLY reader (placeholder sphere emitted)",
            filename or "unknown"))

    else
        -- Unknown shape: emit placeholder
        self:add_node({
            type = "sphere",
            _comment = string.format("TODO: unsupported shape '%s'", shape_type),
        })
        self:add_todo(string.format("Unsupported shape type '%s'", shape_type))
    end
end

function Parser:handle_ObjectBegin()
    local name_tok = self:expect_type("string")
    if not name_tok then return end
    self.current_object_name = name_tok.value
    self.objects[name_tok.value] = {}
    self.transform_stack[#self.transform_stack + 1] = deep_copy(self.current_transforms)
    self.current_transforms = {}
end

function Parser:handle_ObjectEnd()
    self.current_object_name = nil
    self.current_transforms = table.remove(self.transform_stack) or {}
end

function Parser:handle_ObjectInstance()
    local name_tok = self:expect_type("string")
    if not name_tok then return end
    local name = name_tok.value

    local obj_nodes = self.objects[name]
    if not obj_nodes or #obj_nodes == 0 then
        io.stderr:write(string.format("warning: ObjectInstance '%s' not found\n", name))
        return
    end

    -- Deep copy the nodes and add them
    for _, node in ipairs(obj_nodes) do
        self:add_node(deep_copy(node))
    end
end

function Parser:handle_Include()
    local path_tok = self:expect_type("string")
    if not path_tok then return end

    local filepath = self.base_dir .. "/" .. path_tok.value
    local f = io.open(filepath, "r")
    if not f then
        io.stderr:write(string.format("warning: cannot open Include file: %s\n", filepath))
        return
    end
    local content = f:read("*a")
    f:close()

    -- Tokenize and parse the included file
    local inc_tokens = tokenize(content)
    local saved_tokens = self.tokens
    local saved_pos = self.pos

    -- Set base_dir for nested includes relative to the included file
    local inc_dir = filepath:match("(.+)/[^/]+$") or self.base_dir
    local saved_base = self.base_dir
    self.base_dir = inc_dir

    self.tokens = inc_tokens
    self.pos = 1
    self:parse_directives()

    self.tokens = saved_tokens
    self.pos = saved_pos
    self.base_dir = saved_base
end

-- Import is the v4 equivalent of Include (same behavior for our purposes)
Parser.handle_Import = Parser.handle_Include

-- ─────────────────────────────────────────────────────────────────
-- Unsupported directive handlers
-- ─────────────────────────────────────────────────────────────────

--- Directives that take: "type-string" + param list
local directives_string_params = {
    Material = true, MakeNamedMaterial = true,
    LightSource = true, AreaLightSource = true,
    Integrator = true, Sampler = true,
    PixelFilter = true, Accelerator = true,
    CoordSysTransform = true,
}

--- Directives that take: "name-string" only (no params)
local directives_string_only = {
    NamedMaterial = true,
    CoordinateSystem = true,
}

--- Texture: "name" "type" "class" + params (3 strings)
function Parser:handle_Texture()
    local name = self:expect_type("string")
    local ttype = self:expect_type("string")
    local tclass = self:expect_type("string")
    local params = self:read_params()
    local desc = string.format("Texture '%s' (type='%s', class='%s')",
        name and name.value or "?",
        ttype and ttype.value or "?",
        tclass and tclass.value or "?")
    self:add_todo(desc .. " — textures not supported by ART")
end

--- MediumInterface: "interior" "exterior" (2 bare strings, no params)
function Parser:handle_MediumInterface()
    local interior = self:peek()
    if interior and interior.type == "string" then
        self:advance()
    end
    local exterior = self:peek()
    if exterior and exterior.type == "string" then
        self:advance()
    end
    self:add_todo("MediumInterface not supported by ART")
end

--- MakeNamedMedium: "name" + params
function Parser:handle_MakeNamedMedium()
    local name = self:expect_type("string")
    local params = self:read_params()
    self:add_todo(string.format("MakeNamedMedium '%s' not supported by ART",
        name and name.value or "?"))
end

function Parser:handle_ReverseOrientation()
    -- Ignored
end

--- ActiveTransform (v4): consumes identifier (All/StartTime/EndTime)
function Parser:handle_ActiveTransform()
    local t = self:peek()
    if t and t.type == "identifier" then
        self:advance()
    end
end

--- Generic unsupported directive: try to consume "string" + params
function Parser:handle_unsupported_directive(name)
    local t = self:peek()
    if t and t.type == "string" then
        self:advance()
        self:read_params()
    end
    self:add_todo(string.format("Directive '%s' not supported by ART", name))
end

-- ════════════════════════════════════════════════════════════════════
-- Main parse loop
-- ════════════════════════════════════════════════════════════════════

function Parser:parse_directives()
    while self.pos <= #self.tokens do
        local t = self:peek()
        if not t then break end

        if t.type ~= "identifier" then
            self:advance()
        else
            local name = t.value
            self:advance()

            if name == "LookAt" then self:handle_LookAt()
            elseif name == "Camera" then self:handle_Camera()
            elseif name == "Film" then self:handle_Film()
            elseif name == "WorldBegin" then self:handle_WorldBegin()
            elseif name == "WorldEnd" then self:handle_WorldEnd()
            elseif name == "AttributeBegin" then self:handle_AttributeBegin()
            elseif name == "AttributeEnd" then self:handle_AttributeEnd()
            elseif name == "TransformBegin" then self:handle_TransformBegin()
            elseif name == "TransformEnd" then self:handle_TransformEnd()
            elseif name == "Translate" then self:handle_Translate()
            elseif name == "Rotate" then self:handle_Rotate()
            elseif name == "Scale" then self:handle_Scale()
            elseif name == "Identity" then self:handle_Identity()
            elseif name == "ConcatTransform" then self:handle_ConcatTransform()
            elseif name == "Transform" then self:handle_Transform()
            elseif name == "Shape" then self:handle_Shape()
            elseif name == "ObjectBegin" then self:handle_ObjectBegin()
            elseif name == "ObjectEnd" then self:handle_ObjectEnd()
            elseif name == "ObjectInstance" then self:handle_ObjectInstance()
            elseif name == "Include" then self:handle_Include()
            elseif name == "Import" then self:handle_Import()
            elseif name == "Texture" then self:handle_Texture()
            elseif name == "MediumInterface" then self:handle_MediumInterface()
            elseif name == "MakeNamedMedium" then self:handle_MakeNamedMedium()
            elseif name == "ReverseOrientation" then self:handle_ReverseOrientation()
            elseif name == "ActiveTransform" then self:handle_ActiveTransform()
            elseif directives_string_params[name] then
                -- Consume "type" string + params, record TODO
                local type_str = self:peek()
                local type_name = ""
                if type_str and type_str.type == "string" then
                    type_name = type_str.value
                    self:advance()
                end
                self:read_params()
                self:add_todo(string.format("%s '%s' not supported by ART", name, type_name))
            elseif directives_string_only[name] then
                local str_tok = self:peek()
                if str_tok and str_tok.type == "string" then
                    self:advance()
                end
                self:add_todo(string.format("%s not supported by ART", name))
            else
                -- Unknown directive — try generic consumption
                self:handle_unsupported_directive(name)
            end
        end
    end
end

function Parser:parse(source)
    self.tokens = tokenize(source)
    self.pos = 1
    self:parse_directives()
end

-- ════════════════════════════════════════════════════════════════════
-- Lua output generation
-- ════════════════════════════════════════════════════════════════════

local function indent(depth)
    return string.rep("    ", depth)
end

local function fmt_num(v)
    if v == math.floor(v) and math.abs(v) < 1e15 then
        return string.format("%g", v)
    end
    return string.format("%.10g", v)
end

local function fmt_vec(v)
    local parts = {}
    for _, x in ipairs(v) do
        parts[#parts + 1] = fmt_num(x)
    end
    return "{" .. table.concat(parts, ", ") .. "}"
end

--- Emit a transform table at the given indent depth.
local function emit_transform(xf, depth)
    if not xf then return "" end

    local lines = {}
    lines[#lines + 1] = indent(depth) .. "transform = {"

    if xf.translate then
        lines[#lines + 1] = indent(depth + 1) .. "translate = " .. fmt_vec(xf.translate) .. ","
    end
    if xf.rotate then
        lines[#lines + 1] = indent(depth + 1) .. string.format(
            "rotate = {angle = %s, axis = %s},",
            fmt_num(xf.rotate.angle),
            fmt_vec(xf.rotate.axis))
    end
    if xf.scale then
        if type(xf.scale) == "number" then
            lines[#lines + 1] = indent(depth + 1) .. "scale = " .. fmt_num(xf.scale) .. ","
        else
            lines[#lines + 1] = indent(depth + 1) .. "scale = " .. fmt_vec(xf.scale) .. ","
        end
    end
    if xf.matrix then
        lines[#lines + 1] = indent(depth + 1) .. "-- TODO: matrix transform (ART parse_transform does not read 'matrix' yet)"
        lines[#lines + 1] = indent(depth + 1) .. "matrix = " .. fmt_vec(xf.matrix) .. ","
    end

    lines[#lines + 1] = indent(depth) .. "},"
    return table.concat(lines, "\n")
end

local function emit_node(node, depth)
    local lines = {}

    if node._comment then
        lines[#lines + 1] = indent(depth) .. "-- " .. node._comment
    end

    lines[#lines + 1] = indent(depth) .. "{"

    if node.type == "group" then
        lines[#lines + 1] = indent(depth + 1) .. 'type = "group",'
        if node.transform then
            lines[#lines + 1] = emit_transform(node.transform, depth + 1)
        end
        if node.children and #node.children > 0 then
            lines[#lines + 1] = indent(depth + 1) .. "children = {"
            for _, child in ipairs(node.children) do
                lines[#lines + 1] = emit_node(child, depth + 2)
            end
            lines[#lines + 1] = indent(depth + 1) .. "},"
        end

    elseif node.type == "mesh" then
        lines[#lines + 1] = indent(depth + 1) .. 'type = "mesh",'
        if node.transform then
            lines[#lines + 1] = emit_transform(node.transform, depth + 1)
        end

        -- Vertices
        if node.vertices then
            lines[#lines + 1] = indent(depth + 1) .. "vertices = {"
            for _, v in ipairs(node.vertices) do
                lines[#lines + 1] = indent(depth + 2) .. fmt_vec(v) .. ","
            end
            lines[#lines + 1] = indent(depth + 1) .. "},"
        end

        -- Triangles
        if node.triangles then
            lines[#lines + 1] = indent(depth + 1) .. "triangles = {"
            for _, tri in ipairs(node.triangles) do
                lines[#lines + 1] = indent(depth + 2) .. fmt_vec(tri) .. ","
            end
            lines[#lines + 1] = indent(depth + 1) .. "},"
        end

    else
        -- Simple geometry types (sphere, box, plane, disk, cylinder, triangle)
        lines[#lines + 1] = indent(depth + 1) .. string.format('type = "%s",', node.type)
        if node.transform then
            lines[#lines + 1] = emit_transform(node.transform, depth + 1)
        end
    end

    lines[#lines + 1] = indent(depth) .. "},"
    return table.concat(lines, "\n")
end

--- Generate Lua source from a parsed result.
--- @param result table  A table with .scene, .camera_*, .film_*, .todos fields
---                      (as returned by parse_file or parse_source).
--- @return string       Lua source code
local function to_lua(result)
    local lines = {}
    lines[#lines + 1] = "-- Converted from PBRT by pbrt_to_lua.lua"

    -- Emit TODO summary at top if there are unsupported features
    if result.todos and #result.todos > 0 then
        -- Deduplicate
        local seen = {}
        local unique = {}
        for _, msg in ipairs(result.todos) do
            if not seen[msg] then
                seen[msg] = true
                unique[#unique + 1] = msg
            end
        end
        lines[#lines + 1] = "--"
        lines[#lines + 1] = "-- Unsupported PBRT features (require ART implementation):"
        for _, msg in ipairs(unique) do
            lines[#lines + 1] = "--   TODO: " .. msg
        end
    end

    lines[#lines + 1] = ""
    lines[#lines + 1] = "return {"

    -- Render settings
    lines[#lines + 1] = indent(1) .. "render = {"
    lines[#lines + 1] = indent(2) .. "width = " .. result.film_width .. ","
    lines[#lines + 1] = indent(2) .. "height = " .. result.film_height .. ","
    lines[#lines + 1] = indent(2) .. 'output = "' .. result.film_filename .. '",'
    lines[#lines + 1] = indent(2) .. 'accelerator = "bvh",'
    lines[#lines + 1] = indent(1) .. "},"

    -- Camera
    lines[#lines + 1] = ""
    local eye = result.camera_eye or {0, 0, 5}
    local target = result.camera_target or {0, 0, 0}
    local up = result.camera_up or {0, 1, 0}
    lines[#lines + 1] = indent(1) .. "camera = {"
    lines[#lines + 1] = indent(2) .. "position = " .. fmt_vec(eye) .. ","
    lines[#lines + 1] = indent(2) .. "target = " .. fmt_vec(target) .. ","
    lines[#lines + 1] = indent(2) .. "up = " .. fmt_vec(up) .. ","
    if result.camera_fov and result.camera_fov ~= 45 then
        lines[#lines + 1] = indent(2) .. "-- TODO: fov = " .. fmt_num(result.camera_fov)
            .. " (not yet supported by ART Camera)"
    end
    lines[#lines + 1] = indent(1) .. "},"

    -- Scene
    lines[#lines + 1] = ""
    lines[#lines + 1] = indent(1) .. "scene = {"
    for _, node in ipairs(result.scene) do
        lines[#lines + 1] = emit_node(node, 2)
    end
    lines[#lines + 1] = indent(1) .. "},"

    lines[#lines + 1] = "}"
    return table.concat(lines, "\n") .. "\n"
end

-- ════════════════════════════════════════════════════════════════════
-- Module API
-- ════════════════════════════════════════════════════════════════════

--- Parse a PBRT source string.
--- @param source string   PBRT file contents
--- @param opts   table?   Optional: { base_dir = "." }
--- @return table result   Table with .scene, .todos, .camera_*, .film_* fields
function M.parse_source(source, opts)
    opts = opts or {}
    local parser = Parser.new({})
    parser.base_dir = opts.base_dir or "."
    parser:parse(source)
    -- Return a plain result table (not the Parser metatable)
    return {
        scene = parser.scene,
        todos = parser.todos,
        camera_eye = parser.camera_eye,
        camera_target = parser.camera_target,
        camera_up = parser.camera_up,
        camera_fov = parser.camera_fov,
        camera_type = parser.camera_type,
        film_width = parser.film_width,
        film_height = parser.film_height,
        film_filename = parser.film_filename,
        objects = parser.objects,
    }
end

--- Parse a PBRT file from disk.
--- @param path string   Path to a .pbrt file
--- @return table result   (same as parse_source)
--- @return nil           on error: returns nil, error_message
function M.parse_file(path)
    local f, err = io.open(path, "r")
    if not f then return nil, err end
    local source = f:read("*a")
    f:close()
    local base_dir = path:match("(.+)/[^/]+$") or "."
    return M.parse_source(source, { base_dir = base_dir })
end

--- Generate Lua source code from a parsed result.
--- @param result table   As returned by parse_file / parse_source
--- @return string        Lua source code
M.to_lua = to_lua

--- Convenience: parse a PBRT file and write the Lua output.
--- @param input_path  string   Path to input .pbrt file
--- @param output_path string?  Path to output .lua file (nil = return string)
--- @return string|nil          Lua source if output_path is nil
function M.convert_file(input_path, output_path)
    local result, err = M.parse_file(input_path)
    if not result then
        error(string.format("cannot open '%s': %s", input_path, err or "unknown error"))
    end

    local lua_output = M.to_lua(result)

    if output_path then
        local out, werr = io.open(output_path, "w")
        if not out then
            error(string.format("cannot open '%s' for writing: %s", output_path, werr or "unknown"))
        end
        out:write(lua_output)
        out:close()
        io.stderr:write(string.format("Converted %s -> %s\n", input_path, output_path))
        return nil
    end

    return lua_output
end

-- ════════════════════════════════════════════════════════════════════
-- CLI entry point (only when run directly, not when require()'d)
-- ════════════════════════════════════════════════════════════════════

local function main()
    if #arg < 1 then
        io.stderr:write("Usage: lua pbrt_to_lua.lua <input.pbrt> [output.lua]\n")
        os.exit(1)
    end

    local input_path = arg[1]
    local output_path = arg[2] -- optional, defaults to stdout

    if output_path then
        M.convert_file(input_path, output_path)
    else
        local lua_output = M.convert_file(input_path)
        io.write(lua_output)
    end
end

-- Run main() only when executed directly (not when loaded via require())
if arg and arg[0] and arg[0]:match("pbrt_to_lua%.lua$") then
    main()
end

return M
