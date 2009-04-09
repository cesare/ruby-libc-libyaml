require 'mkmf'

LIBDIR = Config::CONFIG['libdir']
INCLUDEDIR = Config::CONFIG['includedir']

$CFLAGS = '-Wall -O2'

HEADER_DIRS = [
  INCLUDEDIR,
  '/opt/local/include',
  '/usr/local/include',
  '/usr/include',
]

LIB_DIRS = [
  LIBDIR,
  '/opt/local/lib',
  '/usr/local/lib',
  '/usr/lib',
]

yaml_dirs = dir_config('libyaml')
unless [nil, nil] == yaml_dirs
  HEADER_DIRS.unshift(yaml_dirs.first)
  LIB_DIRS.unshift(yaml_dirs[1])
end

unless find_header('yaml.h', *HEADER_DIRS)
  abort "yaml.h is missing."
end

unless find_library('yaml', 'yaml_parser_initialize', *LIB_DIRS)
  abort "libyaml is missing."
end

create_makefile('yaml/libyaml')
