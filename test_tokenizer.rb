# THIS IS A MESS

$: << Dir.pwd + '/lib'

require 'benchmark'
require 'nelumbo'
require 'pry'

#t = Nelumbo::Script::Tokenizer.new(File.open('/home/me/Furcadia/Dreams/Spring2011_Final/final version with clearing.ds', 'r'))
t = Nelumbo::Script::Tokenizer.new(File.open('../ch.ds', 'r'))
puts 'Tokenising...'
tokens = t.each_token.to_a
#File.open('tokens.txt', 'w') { |f| tokens.each { |tok| f.puts tok.inspect } }

p = Nelumbo::Script::LineParser.new(Nelumbo::Script::DragonSpeak, tokens)
puts 'Parsing...'
lines = p.lines.to_a
#File.open('lines.txt', 'w') { |f| lines.each { |line| f.puts line.inspect } }

t = Nelumbo::Script::TreeParser.new(Nelumbo::Script::DragonSpeak)
puts 'Running syntax analysis...'
t.parse(lines)

cg = Nelumbo::Script::CodeGenerator.new(Nelumbo::Script::DragonSpeak)
count = 0
t.blocks.each do |blk|
	cg.generate_for blk
	count += 1
	break if count > 20
end

puts cg.finalize

=begin
File.open('tree.txt', 'w') { |f| t.blocks.each { |blk| f.puts blk.inspect } }
puts t.line_count

l = lines.select{|line| line.key?(:category)}
File.open('l.txt', 'w') { |f| l.each { |line| f.puts "#{line[:category]}:#{line[:type]}" } }

l = []
t.blocks.each do |blk|
	blk[:triggers].each do |t|
		l << t[:cause]
		t[:conditions].each { |c| l << c }
	end
	blk[:effects].each { |e| l << e }
end
File.open('t.txt', 'w') { |f| l.each { |line| f.puts "#{line[:category]}:#{line[:type]}" } }
=end

#Pry::start

