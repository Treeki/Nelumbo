# this is a big, big hack

#$LOAD_PATH.unshift File.join(File.dirname($0), 'lib')
require 'nelumbo'

#GC.stress = true

module TestPlugin
	include Nelumbo::Plugin

	when_added do
		puts "Added plugin"
	end

	when_removed do
		puts "Removed plugin"
	end

	on_speech text: 'test' do
		say "Plugin is running!"
	end
end

class TestBot < Nelumbo::Bot
	include Nelumbo::WorldTracking

	on_init_bot do
		register_source_dream 'furc://cypresshomes', '/home/me/Furcadia/Dreams/Spring2011_Final/CHInitialUpload.map'
		register_source_dream 'furc://cypress', '/home/me/Furcadia/Dreams/eep.map'
	end

	set color_code: 't::)5,&(@-&$%#'
	set description: 'Just testing.'

	#on_raw line: /^[^~=>0123678<ABCD]/ do
	#on_raw do
	#	puts data[:line].inspect
	#end
	
	on_speech('pry') { require 'pry'; binding.pry }
	on_speech('load') { add_plugin TestPlugin; say "Loaded plugins: #{plugins}" }
	on_speech('unload') { remove_plugin TestPlugin; say "Loaded plugins: #{plugins}" }
	
	on_connect { puts "Connected!" }
	on_login { puts "Logged in!" }

	on_whisper /^raw / do
		halt unless %w(treeki cypress).include?(data[:shortname])

		write_line data[:text].from(4)
	end

	on_speech('do gc') { GC.start }

	on_speech 'save' do
		path = '/home/me/Furcadia/Dreams/Spring2011_Final/testmap.map'
		path = '/home/me/Furcadia/Dreams/gdfgdfgdfg.map'
		say "Saving current map to #{path}..."
		File.open(path, 'wb') do |f|
			f.write produce_map
		end
		say "Done!"
	end

	on_ds_event 's' do
		say "Someone said s."
	end

	on_ds_event 'output_rand' do
		rand = []
		5.times { |i| rand << ds_var("r#{i+1}") }
		say "Rand: #{rand.map(&:to_s).join(', ')}. Pos: #{ds_var 'tp'},#{ds_var_y 'tp'}"
	end

	on_ds_event 'lotus' do
		puts  "#{context.player.name} just bumped into Lotus!"
	end

	on_ds_event 'water_plant' do
		puts "#{context.player.name} just watered a crop at #{context.trigger_x},#{context.trigger_x}. It started as seed item #{ds_var 'checkseed'} and became #{ds_var 'watered_seed'} on floor type #{ds_var 'checksoil'}."
	end

	on_player_entered { puts "Entered:"; p data }
	on_player_left { puts "Left:"; p data }
	on_player_move { puts "Moved: #{data[:player].name} from #{data[:from_x]},#{data[:from_y]} to #{data[:to_x]},#{data[:to_y]}" }

	on_speech 'start', shortname: 'treeki' do
		say "OK!"
		@is_reporting = true
	end

	on_speech 'stop', shortname: 'treeki' do
		say "OK. :("
		@is_reporting = false
	end

	on_player_entered do
		halt unless @is_reporting
		say "#{data[:player].name} just entered the dream."
	end

	on_player_left do
		halt unless @is_reporting
		say "#{data[:player].name} just left the dream."
	end

	on_player_afk do
		halt unless @is_reporting
		say "#{data[:player].name} just went AFK."
	end

	on_player_unafk do
		halt unless @is_reporting
		say "#{data[:player].name} just came back from being AFK after #{data[:duration]} seconds."
	end

	on_speech /^check / do
		player = find_player!(data[:text].from(6))
		if player.afk?
			say "#{player.name} has been AFK for #{Time.now - player.afk_start_time} seconds."
		else
			say "#{player.name} is not AFK."
		end
	end

	# TODO: Add a :plugin_error event for errors in plugin events.
	# Crash the bot if something breaks in the core, since well... not much we
	# can do :/
end

TestBot.set username: ARGV.first, password: ARGV.last
TestBot.new.run

