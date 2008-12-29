# -*- mode: ruby; coding: utf-8 -*-

require 'rubygems'
require 'rake'
require 'rake/clean'
require 'rake/packagetask'
require 'rake/gempackagetask'
require 'rake/rdoctask'
require 'rake/testtask'
require 'spec/rake/spectask'

dir = File.dirname(__FILE__)
$LOAD_PATH.unshift(File.join(dir, "lib"))

require 'yaml/libyaml'
require 'yaml/libyaml/version'

spec = Gem::Specification.new do |s|
  s.name             = 'yaml-libyaml'
  s.version          = YAML::LibYAML::Version.to_version
  s.author           = 'SAWADA Tadashi'
  s.email            = 'moc.liamg.cesare@gmail.com'
  s.platform         = Gem::Platform::RUBY
  s.summary          = 'Ruby binding module of libyaml library'
  s.homepage         = '' # TODO set later

  files = FileList["{doc,ext,lib,spec}/**/*"].exclude("doc/rdoc").to_a
  files |= ['Rakefile', 'LICENSE']
  s.files            = files
  s.extensions       = ['ext/yaml/extconf.rb']

  s.require_path     = 'lib'
  s.has_rdoc         = true
  s.rdoc_options     = ['--charset', 'UTF-8']
  s.extra_rdoc_files = ['README']
end

if $0 == __FILE__
  Gem::Builder.new(spec).build
end


task :default => ['clean', 'clobber', 'spec', 'rdoc', 'package']


Rake::GemPackageTask.new(spec) do |pkg|
  pkg.need_tar_gz  = true
  pkg.need_tar_bz2 = true
  pkg.need_zip     = true
end


Spec::Rake::SpecTask.new do |t|
  t.spec_files = FileList['spec/**/*_spec.rb']
  t.warning = false
  t.rcov = true
  t.spec_opts = []
  t.rcov_opts = [ '--exclude', 'spec' ]
end

Rake::RDocTask.new('rdoc') do |t|
  t.rdoc_dir = 'doc/rdoc'
  t.rdoc_files.include('README', 'lib/**/*.rb')
  t.main = 'README'
  t.title = 'YAML::LibYAML Documentation'
end
