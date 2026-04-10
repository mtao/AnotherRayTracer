project('AnotherRayTracer', 'cpp',
  version : '0.1',
  default_options : ['warning_level=3', 'cpp_std=c++26'])

# min versions
spdlog_version = '>=1.9.2'

cc = meson.get_compiler('cpp')
dl_lib = cc.find_library('dl')

spdlog_dep = dependency('spdlog', version: spdlog_version, default_options: ['tests=disabled', 'default_library=static'])

zipper_proj = subproject('zipper', default_options: {'testing': false, 'examples': false})
zipper_dep = zipper_proj.get_variable('zipper_dep')

TBB_dep = dependency('tbb')

# ── Quiver (mesh library — required) ─────────────────────────────────────
quiver_proj = subproject('quiver', default_options: {
  'testing': false, 'tools': false, 'examples': false})
quiver_dep = quiver_proj.get_variable('quiver_dep')

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

lib_sources = [
    'src/Point.cpp'
    ,'src/geometry/Box.cpp'
    ,'src/geometry/Sphere.cpp'
    ,'src/geometry/TriangleMesh.cpp'
    ,'src/geometry/Line.cpp'
    ,'src/objects/SceneNode.cpp'
    ,'src/objects/InternalSceneNode.cpp'
    ,'src/objects/Object.cpp'
    ,'src/Camera.cpp'
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
art_lib = library('art', lib_sources,
  include_directories: include_dirs,
  dependencies: internal_deps,
  cpp_args: ['-DART_EXPORTS'] + io_cpp_args,
  gnu_symbol_visibility: 'hidden')

# Full dependency (headers + library linking)
art_dep = declare_dependency(
  link_with: art_lib,
  dependencies: required_deps,
  include_directories: include_dirs)

art_exec = executable('art', 'src/main.cpp', dependencies: [art_dep] + internal_deps)

if get_option('testing')
  subdir('tests')
endif
if get_option('examples')
  subdir('examples')
endif

# ── Optional: terminal viewer tool (requires balsa) ──────────────────────
if get_option('viewer')
  balsa_proj = subproject('balsa', default_options: {
    'testing': false, 'examples': false, 'visualization': false, 'qt': false,
    'glfw': false, 'imgui': false})
  balsaCore_dep = balsa_proj.get_variable('core_dep')

  subdir('tools')
endif
