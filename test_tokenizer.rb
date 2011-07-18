$: << Dir.pwd + '/lib'

require 'benchmark'
require 'nelumbo/script/tokenizer'

t = Nelumbo::Script::Tokenizer.new(File.open('/home/me/Furcadia/Dreams/Spring2011_Final/final version with clearing.ds', 'r'))
puts Benchmark.measure{t.each_token {|t| p t}}

