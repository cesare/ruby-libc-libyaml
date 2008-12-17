require 'mkmf'

dir_config('libyaml')
if have_header('yaml.h') and have_library('yaml')
  create_makefile('yaml/libyaml')
end
