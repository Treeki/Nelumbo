#require 'bundler/gem_tasks'

require 'rubygems'
#require 'rake/dsl_definition'
#require 'rake/alt_system'
require 'rake'
require 'rake/extensiontask'
require 'bundler'

Rake::ExtensionTask.new('nelumbo') do |extension|
	extension.lib_dir = 'lib/nelumbo'
end

task :chmod do
	File.chmod(0775, 'lib/nelumbo/nelumbo.so')
end

task :build => [:clean, :compile, :chmod]

Bundler::GemHelper.install_tasks

