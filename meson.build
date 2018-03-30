project('katachi', ['cpp'],
	license: 'MIT',
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

	# disable some
	# '-Wno-non-virtual-dtor',
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

dep_nytl = dependency('nytl MASTER', fallback: ['nytl', 'nytl_dep'])
dep_dlg = dependency('dlg', fallback: ['dlg', 'dlg_dep'])

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

if get_option('tests')
  bugged_dep = dependency('bugged', fallback: ['bugged', 'bugged_dep'])
  test_deps = [bugged_dep, katachi_dep]

  test_svg = executable('test_svg', 'docs/tests/svg.cpp',
	  dependencies: test_deps)
  test('test_svg', test_svg)
endif
