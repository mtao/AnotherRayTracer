project('stb', 'c',
  version : '0.0.1',
  default_options : ['warning_level=0'])

# stb is a collection of single-file header-only libraries.
# We expose include directories so consumers can #include <stb_image.h> etc.

stb_inc = include_directories('.')

stb_dep = declare_dependency(
  include_directories: stb_inc)
