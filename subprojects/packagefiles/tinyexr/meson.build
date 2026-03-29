project('tinyexr', 'c', 'cpp',
  version : '1.0.9',
  default_options : ['warning_level=0', 'cpp_std=c++17', 'c_std=c11'])

# tinyexr is a single-header library that depends on miniz for
# zlib compression. Both are compiled here into a static library.

tinyexr_inc = include_directories('.', 'deps/miniz')

tinyexr_lib = static_library('tinyexr',
  'tinyexr_impl.cpp',
  'deps/miniz/miniz.c',
  include_directories: tinyexr_inc)

tinyexr_dep = declare_dependency(
  include_directories: tinyexr_inc,
  link_with: tinyexr_lib)
