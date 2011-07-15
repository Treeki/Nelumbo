# this is a big, big hack

$LOAD_PATH.unshift File.join(File.dirname($0), 'lib')
require 'nelumbo'

module TestBot; end

class TestBot::Bot < Nelumbo::Bot
	include Nelumbo::PlayerTracking

	set color_code: 't::)5,&(@-&$%#'
	set description: 'Just testing.'

	on_raw { puts data[:line] }

	on_connect { puts "Connected!" }
	on_login { puts "Logged in!" }

	on_whisper shortname: 'treeki', text: /^raw / do
		write_line data[:text].slice(4..-1)
	end
end

TestBot::Bot.set username: ARGV.first, password: ARGV.last


=begin
class TestBot < Nelumbo::Bot
	on_connect do
		puts 'test'
	end

	on_connect do
		puts 'test again'
	end
end

class TestBot2 < Nelumbo::Bot
	on_connect do
		puts 'test 2'
	end
end

puts 'Should be output: test, test again'
b = TestBot.new
b.dispatch_event :connect

puts 'Should be output: test 2'
c = TestBot2.new
c.dispatch_event :connect

puts 'Should be output: test 2, test 3'

class TestBot2b < TestBot2
	on_connect do
		puts 'test 3'
	end
end
d = TestBot2b.new
d.dispatch_event :connect
=end

