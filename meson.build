project('AnotherRayTracer', 'cpp',
  version : '0.1',
  default_options : ['warning_level=3', 'cpp_std=c++26'])



# min versions
fmt_version = '>=8.0.1'
spdlog_version = '>=1.9.2'



cc = meson.get_compiler('cpp')
dl_lib = cc.find_library('dl')


fmt_dep = dependency('fmt', version: fmt_version)

spdlog_dep = dependency('spdlog', version: spdlog_version, default_options: ['tests=disabled'])

zipper_proj = subproject('zipper', default_options: {'testing': false, 'examples': false})
zipper_dep = zipper_proj.get_variable('zipper_dep')

TBB_dep = dependency('tbb')

required_deps = [zipper_dep]
internal_deps = [spdlog_dep, fmt_dep] + required_deps

lib_sources = [
    'src/Point.cpp'

    ,'src/geometry/Box.cpp'
    ,'src/geometry/Sphere.cpp'

    ,'src/objects/SceneNode.cpp'
    ,'src/objects/InternalSceneNode.cpp'
    ,'src/objects/Object.cpp'

    ,'src/utils/AffineTransform.cpp'

    ,'src/Camera.cpp'
    ,'src/geometry/Line.cpp'
    ,'src/Rational.cpp'
    ,'src/Ray.cpp'
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
  cpp_args: ['-DART_EXPORTS'],
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
