# -*- encoding: utf-8 -*-
require 'rake'
$:.push File.expand_path("../lib", __FILE__)

Gem::Specification.new do |s|
  s.name        = "nelumbo"
  s.version     = '0.0.1'
  s.authors     = ["Treeki"]
  s.email       = ["treeki@gmail.com"]
  s.homepage    = ""
  s.summary     = %q{Framework for creating bots and handling data files for the online MMOSG Furcadia.}
  s.description = <<END
Framework for creating bots and handling data files for the online MMOSG Furcadia. Todo: Add more here.
END

  #s.rubyforge_project = "nelumbo"

  s.files         = Dir['**/*']
  s.test_files         = Dir['spec/**/*']
  s.executables   = []
  s.require_paths = ["lib"]

  s.extensions << "ext/nelumbo/extconf.rb"

  s.required_ruby_version = '>=1.9'

  s.add_dependency('i18n')
  s.add_dependency('eventmachine')
  s.add_dependency('mixology')
  s.add_dependency('activesupport', '~> 3.2.17')
  s.add_dependency('rspec-core')
  s.add_dependency('rspec-expectations')
  s.add_dependency('rspec-mocks')
  s.add_dependency('bindata')
end
