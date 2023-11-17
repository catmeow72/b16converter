project('graphicsconverter', 'cpp',
  version : '0.1',
  default_options : ['warning_level=3',
                     'cpp_std=c++14'])

deps = [
	dependency('Magick++', version : '>=7.0')
]
srcs = [
	'palette.cpp',
	'bitmapx16.cpp',
	'main.cpp'
]
exe = executable('b16converter', srcs,
  install : true,
  dependencies : deps)

test('basic', exe)
