project('katachi', ['cpp'],
	license: 'BSL-1.0',
	version: '0.1.0',
	meson_version: '>=0.43',
	default_options: [
		'cpp_std=c++1z',
  		'warning_level=3',
  		'werror=true'])

warnings = [
	'-Wall',
	'-Wextra',
	'-Wpedantic',

	# extra
	'-Wduplicated-cond',
	'-Wrestrict',
	'-Wnull-dereference',
	'-Wundef',
	'-Wcast-align=strict',
	'-Wlogical-op',
]

# default arguments
cc = meson.get_compiler('cpp')
add_project_arguments(
	cc.get_supported_arguments(warnings),
	language: 'cpp')

# project-specific stuff
source_root = meson.current_source_dir().split('\\')
dlg_path_arg = '-DDLG_BASE_PATH="' + '/'.join(source_root) + '/"'
add_project_arguments(dlg_path_arg, language: 'cpp')
add_project_arguments('-DDLG_DEFAULT_TAGS="katachi",', language: 'cpp')

dep_nytl = dependency('nytl',
	version: '>=0.6.0',
	fallback: ['nytl', 'nytl_dep'])

dep_dlg = dependency('dlg',
	version: '>=0.2.2',
	fallback: ['dlg', 'dlg_dep'])

# build
katachi_inc = include_directories('include')
katachi_deps = [
  dep_nytl,
  dep_dlg,
]

katachi_src = [
  'src/katachi/path.cpp',
  'src/katachi/stroke.cpp',
  'src/katachi/curves.cpp',
  'src/katachi/svg.cpp',
]

katachi_lib = library('katachi',
	sources: katachi_src,
	dependencies: katachi_deps,
	include_directories: [katachi_inc])

katachi_dep = declare_dependency(
	link_with: katachi_lib,
	dependencies: [dep_nytl],
	include_directories: katachi_inc)

# tests
if get_option('tests')
  dep_bugged = dependency('bugged', fallback: ['bugged', 'bugged_dep'])
  test_deps = [dep_bugged, dep_dlg, katachi_dep]

  test_svg = executable('test_svg', 'docs/tests/svg.cpp',
	  dependencies: test_deps)
  test('test_svg', test_svg)
endif

# pkgconfig
pkg = import('pkgconfig')
pkg_dirs = ['.', 'katachi']
pkg.generate(
	name: 'katachi',
	filebase: 'katachi',
	requires: ['nytl'],
	subdirs: pkg_dirs,
	version: meson.project_version(),
	description: 'Curve and path baking')
