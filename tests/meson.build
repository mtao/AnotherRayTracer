catch2_with_main_dep = dependency('catch2-with-main', default_options: ['tests=false'])

art_test_dep = declare_dependency(
  dependencies: [art_dep, catch2_with_main_dep])

sources = ['test_point.cpp', 'test_rational.cpp', 'test_image.cpp', 'test_image_io.cpp', 'test_intersection.cpp', 'test_plane.cpp', 'test_disk.cpp', 'test_cylinder.cpp', 'test_triangle.cpp', 'test_mesh_geometry.cpp', 'test_accelerator.cpp']
test_art = executable('test_art', sources, dependencies: [art_test_dep])
test('art', test_art)
