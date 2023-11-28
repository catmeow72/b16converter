project('graphicsconverter', ['cpp', 'c'],
  version : '0.1',
  default_options : ['warning_level=3',
                     'cpp_std=c++17',
                     'default_library=static'])

deps = [
	dependency('Magick++', version : '>=6.9.11'),
	dependency('openmp')
]
srcs = [
	'palette.cpp',
	'bitmapx16.cpp',
	'main.cpp',
	'lzsa/src/dictionary.c',
	'lzsa/src/expand_block_v1.c',
	'lzsa/src/expand_block_v2.c',
	'lzsa/src/expand_context.c',
	'lzsa/src/expand_inmem.c',
	'lzsa/src/expand_streaming.c',
	'lzsa/src/frame.c',
	'lzsa/src/matchfinder.c',
	'lzsa/src/shrink_block_v1.c',
	'lzsa/src/shrink_block_v2.c',
	'lzsa/src/shrink_context.c',
	'lzsa/src/shrink_inmem.c',
	'lzsa/src/shrink_streaming.c',
	'lzsa/src/stream.c',
	'lzsa/src/libdivsufsort/lib/divsufsort_utils.c',
	'lzsa/src/libdivsufsort/lib/divsufsort.c',
	'lzsa/src/libdivsufsort/lib/sssort.c',
	'lzsa/src/libdivsufsort/lib/trsort.c'
]
incdirs = [
	'lzsa/src',
	'lzsa/src/libdivsufsort/include'
]
exe = executable('bmxconverter', srcs,
  install : true,
  include_directories: incdirs,
  c_args: ['-DHAVE_CONFIG_H=1','-D__STDC_LIMIT_MACROS','-D__STDC_CONSTANT_MACROS','-D__STDC_FORMAT_MACROS'],
  dependencies : deps)

test('basic', exe)
