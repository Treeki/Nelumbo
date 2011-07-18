$: << Dir.pwd + '/lib'

require 'benchmark'
require 'nelumbo/script/tokenizer'
require 'nelumbo/script/parser'
require 'nelumbo/script/language'
require 'nelumbo/script/dragon_speak'

t = Nelumbo::Script::Tokenizer.new(File.open('/home/me/Furcadia/Dreams/Spring2011_Final/final version with clearing.ds', 'r'))
tokens = t.each_token.to_a
File.open('tokens.txt', 'w') { |f| tokens.each { |tok| f.puts tok.inspect } }

p = Nelumbo::Script::LineParser.new(Nelumbo::Script::DragonSpeak, tokens)
File.open('output.txt', 'w') { |f| p.each_line { |line| f.puts line.inspect } }

