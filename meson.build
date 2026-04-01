project('AnotherRayTracer', 'cpp',
  version : '0.1',
  default_options : ['warning_level=3', 'cpp_std=c++26'])



# min versions
spdlog_version = '>=1.9.2'



spdlog_dep = dependency('spdlog', version: spdlog_version, default_options: ['tests=disabled'])

zipper_dep = dependency('zipper', default_options: ['testing=false', 'examples=false'])

# Determine lua option early so quiver can be built with lua support if needed.
lua_enabled = get_option('lua')

quiver_opts = [
    'testing=false',
    'examples=false',
    'tools=false',
]
if lua_enabled
  quiver_opts += 'lua=true'
else
  quiver_opts += 'lua=false'
endif

quiver_dep = dependency('quiver', default_options: quiver_opts)

required_deps = [zipper_dep, quiver_dep]
internal_deps = [spdlog_dep] + required_deps

# Optional image I/O dependencies
io_cpp_args = []

if get_option('exr')
  tinyexr_proj = subproject('tinyexr')
  tinyexr_dep = tinyexr_proj.get_variable('tinyexr_dep')
  internal_deps += tinyexr_dep
  io_cpp_args += '-DART_HAS_EXR=1'
endif

if get_option('png')
  stb_proj = subproject('stb')
  stb_dep = stb_proj.get_variable('stb_dep')
  internal_deps += stb_dep
  io_cpp_args += '-DART_HAS_PNG=1'
endif

# Optional: sol2 Lua bindings for scene description.
# sol2's meson.build pulls lua as a dependency internally.
# lua_enabled was determined above (before quiver dep).
lua_sources = []
lua_deps = []
lua_cpp_args = []
if lua_enabled
  sol2_proj = subproject('sol2')
  sol2_dep = sol2_proj.get_variable('sol2_dep')
  lua_deps += [sol2_dep]
  lua_cpp_args += '-DART_HAS_LUA=1'
  lua_sources += [
      'src/lua/bindings.cpp',
      'src/lua/bind_geometry.cpp',
      'src/lua/bind_scene.cpp',
      'src/lua/bind_camera.cpp',
      'src/lua/bind_image.cpp',
      'src/lua/bind_transform.cpp',
      'src/lua/scene_loader.cpp',
      'src/lua/scene_serializer.cpp',
  ]
endif

lib_sources = [
    'src/Point.cpp'

    ,'src/geometry/Box.cpp'
    ,'src/geometry/Sphere.cpp'
    ,'src/geometry/Plane.cpp'
    ,'src/geometry/Disk.cpp'
    ,'src/geometry/Cylinder.cpp'
    ,'src/geometry/Triangle.cpp'
    ,'src/geometry/MeshGeometry.cpp'
    ,'src/geometry/mesh_utils.cpp'

    ,'src/objects/SceneNode.cpp'
    ,'src/objects/InternalSceneNode.cpp'
    ,'src/objects/Object.cpp'

    ,'src/accel/SceneAccelerator.cpp'
    ,'src/accel/intersect_primitive.cpp'
    ,'src/accel/LinearAccelerator.cpp'
    ,'src/accel/BVHAccelerator.cpp'

    ,'src/Camera.cpp'
    ,'src/geometry/Line.cpp'
    ,'src/Rational.cpp'
    ,'src/Ray.cpp'
    ,'src/Image.cpp'
    ,'src/io/image_io.cpp'
    ,'src/io/ppm.cpp'
    ,'src/io/exr.cpp'
    ,'src/io/png.cpp'
]

include_dirs = [include_directories('include')]

# Header-only dependency (for targets that only need headers)
art_headers_dep = declare_dependency(
  dependencies: required_deps,
  include_directories: include_dirs)

# Compiled library
art_lib = library('art', lib_sources + lua_sources,
  include_directories: include_dirs,
  dependencies: internal_deps + lua_deps,
  cpp_args: ['-DART_EXPORTS'] + io_cpp_args + lua_cpp_args,
  gnu_symbol_visibility: 'hidden')

# Full dependency (headers + library linking)
art_dep = declare_dependency(
  link_with: art_lib,
  dependencies: required_deps,
  include_directories: include_dirs)
meson.override_dependency('art', art_dep)
meson.override_dependency('art-headers', art_headers_dep)


art_exec = executable('art', 'src/main.cpp', dependencies: [art_dep] + internal_deps)

if lua_enabled
  art_lua_exec = executable('art-lua', 'src/art_lua.cpp',
    dependencies: [art_dep] + internal_deps + lua_deps,
    cpp_args: lua_cpp_args)
endif


if get_option('tools')
  subdir('tools')
endif
if get_option('testing')
  subdir('tests')
endif
if get_option('examples')
  subdir('examples')
endif
