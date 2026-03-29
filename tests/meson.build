catch2_with_main_dep = dependency('catch2-with-main', default_options: ['tests=false'])

art_test_dep = declare_dependency(
  dependencies: [art_dep, catch2_with_main_dep])

sources = ['test_point.cpp', 'test_rational.cpp', 'test_image.cpp', 'test_image_io.cpp', 'test_intersection.cpp', 'test_plane.cpp', 'test_disk.cpp', 'test_cylinder.cpp', 'test_triangle.cpp', 'test_mesh_geometry.cpp', 'test_accelerator.cpp']
test_art = executable('test_art', sources, dependencies: [art_test_dep])
test('art', test_art)

if lua_enabled
  lua_test_sources = ['test_scene_loader.cpp']
  test_art_lua = executable('test_art_lua', lua_test_sources,
    dependencies: [art_test_dep] + lua_deps,
    cpp_args: lua_cpp_args)
  test('art_lua', test_art_lua)
endif
