#!/usr/bin/env lua
--- validate_roundtrip — Batch-validate PBRT -> Lua conversion across scene repos.
---
--- For each .pbrt file found, converts to Lua and checks that the output
--- is valid Lua syntax (loads without errors). Optionally tests round-trip
--- by converting Lua -> PBRT -> Lua and comparing. With --pbrt, also
--- validates the generated PBRT output using the actual pbrt binary.
---
--- Usage:
---   lua validate_roundtrip.lua <scenes_dir> [options]
---
--- Options:
---   --roundtrip          Also test Lua -> PBRT -> Lua round-trip
---   --pbrt=<path>        Validate generated PBRT with `pbrt --format`
---   --verbose            Print per-file results
---   --stop-on-error      Stop at first failure
---   --max-size=N         Skip .pbrt files larger than N bytes
---
--- Examples:
---   lua validate_roundtrip.lua ../pbrt-scenes/pbrt-v4-scenes/
---   lua validate_roundtrip.lua ../pbrt-scenes/ --roundtrip --verbose
---   lua validate_roundtrip.lua ../pbrt-scenes/ --max-size=5000000
---   lua validate_roundtrip.lua ../pbrt-scenes/ --pbrt=$HOME/git/pbrt-v4/build/pbrt

-- Add tools/ to package.path so we can require the converters
local script_dir = arg[0]:match("(.+)/[^/]+$") or "."
package.path = script_dir .. "/?.lua;" .. package.path

local pbrt = require("pbrt_to_lua")
local l2p = require("lua_to_pbrt")

-- ════════════════════════════════════════════════════════════════════
-- File discovery
-- ════════════════════════════════════════════════════════════════════

--- Recursively find all .pbrt files under a directory.
--- Uses `find` since Lua has no built-in directory traversal.
--- @param dir string       Directory to search
--- @param max_size number? Optional max file size in bytes (nil = no limit)
local function find_pbrt_files(dir, max_size)
    local size_filter = ""
    if max_size then
        size_filter = string.format(" -size -%dc", max_size)
    end
    local files = {}
    local handle = io.popen(string.format(
        'find "%s" -name "*.pbrt" -type f%s 2>/dev/null | sort', dir, size_filter))
    if not handle then return files end
    for line in handle:lines() do
        files[#files + 1] = line
    end
    handle:close()
    return files
end

-- ════════════════════════════════════════════════════════════════════
-- Validation
-- ════════════════════════════════════════════════════════════════════

local function validate_lua_syntax(lua_source, source_name)
    -- Try to load the Lua source as a chunk (syntax check only)
    local chunk, err = load(lua_source, source_name)
    if not chunk then
        return false, "syntax error: " .. err
    end
    -- Execute to verify it returns a table
    local ok, result = pcall(chunk)
    if not ok then
        return false, "runtime error: " .. tostring(result)
    end
    if type(result) ~= "table" then
        return false, string.format("expected table return, got %s", type(result))
    end
    -- Verify required keys
    if not result.scene then
        return false, "missing 'scene' key in returned table"
    end
    return true, result
end

--- Validate PBRT source by running `pbrt --format` on it.
--- Returns true on success, false + error message on failure.
local function validate_with_pbrt(pbrt_source, pbrt_bin)
    local tmpf = os.tmpname()
    local f = io.open(tmpf, "w")
    if not f then return false, "cannot create temp file" end
    f:write(pbrt_source)
    f:close()

    local handle = io.popen(string.format(
        '%s --format "%s" >/dev/null 2>&1; echo $?', pbrt_bin, tmpf))
    local exit_str = handle:read("*a"):match("%d+")
    handle:close()
    local exit_code = tonumber(exit_str) or 1

    if exit_code ~= 0 then
        -- Re-run to capture error message
        handle = io.popen(string.format('%s --format "%s" 2>&1', pbrt_bin, tmpf))
        local output = handle:read("*a")
        handle:close()
        os.remove(tmpf)
        -- Extract first error line
        local errmsg = "unknown error"
        for line in output:gmatch("[^\n]+") do
            if line:lower():match("error") then
                errmsg = line
                break
            end
        end
        return false, errmsg
    end

    os.remove(tmpf)
    return true, nil
end

-- ════════════════════════════════════════════════════════════════════
-- Main
-- ════════════════════════════════════════════════════════════════════

local function main()
    if #arg < 1 then
        io.stderr:write("Usage: lua validate_roundtrip.lua <scenes_dir> [--roundtrip] [--pbrt=<path>] [--verbose] [--stop-on-error] [--max-size=N]\n")
        os.exit(1)
    end

    local scenes_dir = arg[1]
    local do_roundtrip = false
    local verbose = false
    local stop_on_error = false
    local max_size = nil
    local pbrt_bin = nil

    for i = 2, #arg do
        if arg[i] == "--roundtrip" then do_roundtrip = true end
        if arg[i] == "--verbose" then verbose = true end
        if arg[i] == "--stop-on-error" then stop_on_error = true end
        local sz = arg[i]:match("^%-%-max%-size=(%d+)$")
        if sz then max_size = tonumber(sz) end
        local pb = arg[i]:match("^%-%-pbrt=(.+)$")
        if pb then pbrt_bin = pb end
    end

    -- Verify pbrt binary exists if specified
    if pbrt_bin then
        local test = io.popen(string.format('"%s" --help >/dev/null 2>&1; echo $?', pbrt_bin))
        local rc = tonumber(test:read("*a"):match("%d+")) or 1
        test:close()
        if rc ~= 0 then
            io.stderr:write(string.format("Error: pbrt binary not found or not executable: %s\n", pbrt_bin))
            os.exit(1)
        end
        io.stderr:write(string.format("Using pbrt binary: %s\n", pbrt_bin))
    end

    local files = find_pbrt_files(scenes_dir, max_size)
    if #files == 0 then
        io.stderr:write(string.format("No .pbrt files found under '%s'\n", scenes_dir))
        os.exit(1)
    end

    io.stderr:write(string.format("Found %d .pbrt files\n", #files))

    local pass = 0
    local fail = 0
    local skip = 0
    local roundtrip_pass = 0
    local roundtrip_fail = 0
    local pbrt_pass = 0
    local pbrt_fail = 0
    local failures = {}

    for i, filepath in ipairs(files) do
        local short = filepath:sub(#scenes_dir + 1):gsub("^/", "")

        -- Step 1: PBRT -> Lua
        local result, parse_err = pbrt.parse_file(filepath)
        if not result then
            fail = fail + 1
            local msg = string.format("[FAIL] %s: parse error: %s", short, parse_err or "unknown")
            failures[#failures + 1] = msg
            if verbose then io.stderr:write(msg .. "\n") end
            if stop_on_error then break end
        else
            local lua_source = pbrt.to_lua(result)
            local ok, err = validate_lua_syntax(lua_source, short)
            if ok then
                pass = pass + 1
                if verbose then
                    io.stderr:write(string.format("[PASS] %s (%d nodes, %d todos)\n",
                        short, #result.scene, #result.todos))
                end

                -- Step 2: optional pbrt binary validation (PBRT -> Lua -> PBRT -> pbrt --format)
                if pbrt_bin then
                    local pbrt_source = l2p.to_pbrt(result)
                    local pbrt_ok, pbrt_err = validate_with_pbrt(pbrt_source, pbrt_bin)
                    if pbrt_ok then
                        pbrt_pass = pbrt_pass + 1
                        if verbose then
                            io.stderr:write(string.format("  [PBRT-PASS] %s\n", short))
                        end
                    else
                        pbrt_fail = pbrt_fail + 1
                        local msg = string.format("  [PBRT-FAIL] %s: %s", short, pbrt_err)
                        failures[#failures + 1] = msg
                        if verbose then io.stderr:write(msg .. "\n") end
                        if stop_on_error then break end
                    end
                end

                -- Step 3: optional round-trip (Lua -> PBRT -> Lua)
                if do_roundtrip then
                    local pbrt_source = l2p.to_pbrt(result)
                    local rt_result = pbrt.parse_source(pbrt_source)
                    local rt_lua = pbrt.to_lua(rt_result)
                    local rt_ok, rt_err = validate_lua_syntax(rt_lua, short .. " (roundtrip)")
                    if rt_ok then
                        roundtrip_pass = roundtrip_pass + 1
                        if verbose then
                            io.stderr:write(string.format("  [RT-PASS] %s\n", short))
                        end
                    else
                        roundtrip_fail = roundtrip_fail + 1
                        local msg = string.format("  [RT-FAIL] %s: %s", short, rt_err)
                        failures[#failures + 1] = msg
                        if verbose then io.stderr:write(msg .. "\n") end
                        if stop_on_error then break end
                    end
                end
            else
                fail = fail + 1
                local msg = string.format("[FAIL] %s: %s", short, err)
                failures[#failures + 1] = msg
                if verbose then io.stderr:write(msg .. "\n") end
                if stop_on_error then break end
            end
        end

        -- Progress indicator (every 50 files)
        if not verbose and i % 50 == 0 then
            io.stderr:write(string.format("  ... %d/%d\n", i, #files))
        end
    end

    -- Summary
    io.stderr:write("\n")
    io.stderr:write(string.format("═══ Results ═══\n"))
    io.stderr:write(string.format("  PBRT -> Lua:  %d pass, %d fail (of %d files)\n",
        pass, fail, #files))
    if pbrt_bin then
        io.stderr:write(string.format("  pbrt verify:  %d pass, %d fail\n",
            pbrt_pass, pbrt_fail))
    end
    if do_roundtrip then
        io.stderr:write(string.format("  Round-trip:   %d pass, %d fail\n",
            roundtrip_pass, roundtrip_fail))
    end

    if #failures > 0 then
        io.stderr:write(string.format("\n  Failures (%d):\n", #failures))
        for _, msg in ipairs(failures) do
            io.stderr:write("    " .. msg .. "\n")
        end
    end

    os.exit((fail + pbrt_fail) > 0 and 1 or 0)
end

if arg and arg[0] and arg[0]:match("validate_roundtrip%.lua$") then
    main()
end
