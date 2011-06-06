# -*- encoding: utf-8 -*-
$:.push File.expand_path("../lib", __FILE__)

Gem::Specification.new do |s|
  s.name        = "nelumbo"
  s.version     = '0.0.1'
  s.authors     = ["Treeki"]
  s.email       = ["treeki@gmail.com"]
  s.homepage    = ""
  s.summary     = %q{TODO: Write a gem summary}
  s.description = %q{TODO: Write a gem description}

  #s.rubyforge_project = "nelumbo"

  s.files         = `git ls-files`.split("\n")
  s.test_files    = `git ls-files -- {test,spec,features}/*`.split("\n")
  s.executables   = `git ls-files -- bin/*`.split("\n").map{ |f| File.basename(f) }
  s.require_paths = ["lib"]

  s.required_ruby_version = '>=1.9'

  s.add_dependency('i18n')
  s.add_dependency('activesupport')
  s.add_dependency('rspec-core')
  s.add_dependency('rspec-expectations')
  s.add_dependency('rspec-mocks')
end
