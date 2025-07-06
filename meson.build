project('AnotherRayTracer', 'cpp',
  version : '0.1',
  default_options : ['warning_level=3', 'cpp_std=c++23'])




# min versions
fmt_version = '>=8.0.1'
spdlog_version = '>=1.9.2'



cc = meson.get_compiler('cpp')
dl_lib = cc.find_library('dl')


fmt_dep = dependency('fmt', version: fmt_version)

spdlog_dep = dependency('spdlog', version: spdlog_version)

zipper_proj = subproject('zipper', default_options: {'testing': false, 'examples': false})
zipper_dep = zipper_proj.get_variable('zipper_dep')

range_v3_dep = dependency('range-v3')



TBB_dep = dependency('tbb')



range_v3_dep = dependency('range-v3')

required_deps = [range_v3_dep, zipper_dep]
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


art_lib = library('art', lib_sources, include_directories: include_dirs, dependencies: internal_deps)
art_dep = declare_dependency(link_with: art_lib, dependencies: required_deps, include_directories: include_dirs)


art_exec = executable('art', 'src/main.cpp', dependencies: [art_dep] + internal_deps)


if get_option('testing')
  subdir('tests')
endif
if get_option('examples')
  subdir('examples')
endif
