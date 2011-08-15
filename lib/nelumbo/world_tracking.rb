module Nelumbo
	# This module allows a bot to track the world around it. It has different
	# features depending on whether the C extensions are loaded or not.
	#
	# == Without Extensions
	# A pure-Ruby implementation is used. It tracks players in and around the
	# dream using the avatar/movement info.
	#
	# == With Extensions
	# A full DragonSpeak engine (for dreams where the source files are
	# available) is implemented, in addition to the aforementioned feature.
	#
	# To register a dream's source files with the bot, the
	# register_source_dream method is called. Each dream URL is associated with
	# one source dream. The amount of DS lines reported by the server is
	# checked against the DS file to try and make sure it's the correct dream
	# (although obviously this won't be foolproof).
	#
	# === DS Engine Caveats
	# This engine is capable of tracking almost all changes in the dream (both
	# player and map wise). It can even track players around the dream
	# accurately, with a couple of caveats preventing it from being 100%
	# perfect:
	# - The (5:15) and (5:17) DS lines cannot be used, since the server
	#   randomly generates a target position and doesn't tell the client
	#   what it is.
	# - The (5:19) line cannot be used, since it needs to know the direction
	#   that "any furre present" is facing and the server only divulges this
	#   information for the triggering furre and visible players.
	# - Code which relies on areas iterating through positions in a specific
	#   order (which probably only affects Move/Copy/Swap Object/Floor/Wall)
	#   may not work correctly, because I haven't figured out the exact
	#   orders that Furcadia uses and I don't want to mess around with
	#   diamonds any more.
	# - Full copies of the original map, DS and FOX files are required, since
	#   third-party applications are forbidden from decrypting dreams (for good
	#   reason) and I don't want to implement the file server protocol anyway.
	#
	# === DragonSpeak Annotations
	# An annotation is a Nelumbo-specific instruction inserted into the DS
	# file. Each line can have one annotation attached to it.
	#
	# To specify one, add a comment starting with [NB] directly above the
	# corresponding DS line. For example:
	#   *[NB] event do_something_cool
	#   (5:200) emit message {Something cool is being done!}.
	#
	# The current version of Nelumbo only supports one kind of annotation.
	# All others are ignored.
	#
	# [event _name_]
	#   Raises +:ds_event+ and passes the specified event name when the
	#   associated line is executed. It currently only works with effects:
	#   this restriction will be lifted later.
	#
	# === Note about X Positions
	# Furcadia internally stores X positions halved - for example, a 300x300
	# map is really a 150x300 map. You will never see an odd X position unless
	# you are dealing with walls.
	#
	# All public API methods accept and return doubled X positions (as used in
	# DragonSpeak and DreamEd), including Player#x.
	#
	# For the C implementation, it's a little more complicated. Every internal
	# function uses halved positions, but those accessible through Ruby methods
	# use doubled positions.
	#
	# == Events
	# WorldTracking will raise these events:
	# [player_entered]
	#   A player entered the dream.
	#   Data: +:shortname+, +:player+, +:uid+, +:flags+
	# [player_left]
	#   A player left the dream.
	#   Data: +:shortname+, +:player+, +:uid+
	# [player_seen]
	#   A player was seen. This occurs when the "spawn avatar" instruction is
	#   received, but that player is already in the dream.
	#   Data: +:shortname+, +:player+, +:uid+, +:flags+
	# [player_move]
	#   A player moved. The bot will trigger this whenever it finds out about a
	#   position change. (Sources include the avatar info lines, DS trigger
	#   positions and others.)
	#   Data: +:player+, +:from_x+, +:from_y+, +:to_x+, +:to_y+
	# [player_afk]
	#   A player went AFK.
	#   Data: +:player+
	# [player_unafk]
	#   A player came back from being AFK.
	#   Data: +:player+, +:duration+ (in seconds)
	#
	# WorldTracking will raise these events if the DS engine is active:
	# [ds_event]
	#   A DragonSpeak event was raised. See the section on annotations above.
	#   Data: +:name+ (name of the event that was raised)
	#
	module WorldTracking
		module SimpleImplementation
			class Player
				def initialize(uid, name)
					@uid = uid
					@name = name
					@shortname = name.to_shortname
				end

				attr_reader :uid, :shortname
				attr_accessor :name, :color_code
				attr_accessor :x, :y, :visible, :afk_start_time

				# transient info that may or may not be up-to-date
				attr_accessor :entry_code, :shape, :held_object, :cookies

				def afk?
					!@afk_start_time.nil?
				end

				def afk_length
					Time.now - @afk_start_time
				end
			end

			module ClassMethods
				def setup_world_tracking
					super

					on_init_bot do
						@player_list = []
						@player_lookup_by_uid = {}
						@player_lookup_by_shortname = {}
						@player_lookup_by_position = {}
					end
				end
			end


			def reset_world_tracking
				@player_list.clear
				@player_lookup_by_uid.clear
				@player_lookup_by_shortname.clear
				@player_lookup_by_position.clear
			end


			def create_and_add_player(uid, name)
				player = Player.new(uid, name)

				@player_list << player
				@player_lookup_by_uid[uid] = player
				@player_lookup_by_shortname[player.shortname] = player

				player
			end

			def delete_and_remove_player(player)
				@player_list.delete player
				@player_lookup_by_uid.delete player.uid
				@player_lookup_by_shortname.delete player.shortname
				@player_lookup_by_position.delete (player.x << 11) | player.y
			end

			# Returns a Player matching the specified user ID
			def find_player_by_uid(uid)
				@player_lookup_by_uid[uid]
			end

			# Returns a Player matching the specified name, taking
			# shortnames/longnames into account
			def find_player_by_name(name)
				@player_lookup_by_shortname[name.to_shortname]
			end

			# Returns a Player at the specified position
			def find_player_at_position(x, y)
				# Note: we shift by 11 and not 12 because the X value is multiplied by 2
				@player_lookup_by_position[(x << 11) | y]
			end

			# @private
			# Moves a player to a position. This *MUST* be used or
			# find_player_at_position will not work correctly! This method
			# accepts a doubled X position.
			def move_tracked_player(player, x, y)
				return if player.x == x and player.y == y

				old_x = player.x
				old_y = player.y

				@player_lookup_by_position.delete (old_x << 11) | old_y unless old_x.nil?
				player.x = x
				player.y = y
				@player_lookup_by_position[(x << 11) | y] = player

				# new players start at 0,0, as with DS
				if old_x.nil?
					old_x = 0
					old_y = 0
				end

				dispatch_event :player_move, player: player, from_x: old_x, from_y: old_y, to_x: x, to_y: y
			end
		end

		module AdvancedImplementation
			attr_reader :context

			def reset_world_tracking
				@context = Nelumbo::World::Context.new(self)

				write_line 'dreambookmark 0'

				@world_ds_count = -1
				@world_waiting_for_url = true
			end

			module ClassMethods
				def setup_world_tracking
					super

					set do_not_send_vascodagama: true

					on_init_bot do
						init_source_dream_list
					end

					on_message line: /^<img src='fsh:\/\/system\.fsh:86' \/> Lines of DragonSpeak/ do
						@world_ds_count = data[:line][/Speak: (\d+)/,1].to_i
					end

					on_raw line: /^\]C0/ do
						halt unless @world_waiting_for_url

						@world_waiting_for_url = false
						init_world_engine(data[:line].from(3), @world_ds_count)
						write_line 'vascodagama'
					end

					on_raw line: /^[>0123678]/ do
						# this bit is done in C code to save some time.
						# every little helps, right?
						@context.process_line(data[:line])
					end
				end
			end


			def create_and_add_player(uid, name)
				@context.create_and_add_player(uid, name)
			end

			def delete_and_remove_player(player)
				@context.delete_and_remove_player(player)
			end

			def find_player_by_uid(uid); @context.find_player_by_uid(uid); end
			def find_player_by_name(name); @context.find_player_by_name(name); end
			def find_player_at_position(x, y); @context.find_player_at_position(x, y); end

			def each_player(&block); @context.each_player(&block); end
			def move_tracked_player(player, x, y); @context.move_tracked_player(player, x, y); end

			def normalise_dream_url(url)
				# TODO: make this better
				url.downcase.gsub(%r(/$), '')
			end

			def map_directory
				@current_map_directory
			end

			def map_path
				@current_map_path
			end

			def init_source_dream_list
				@world_source_dreams = {}
			end

			def register_source_dream(url, path)
				@world_source_dreams[normalise_dream_url(url)] = path
			end

			def init_world_engine(url, line_count)
				puts "Initialising World Tracking for #{url}."

				dream_path = @world_source_dreams[normalise_dream_url(url)]
				if dream_path.nil?
					puts "No source path is registered for this dream."
					return
				end

				@current_map_path = dream_path
				@current_map_directory = File.dirname(dream_path)

				# TODO: Abstract this into a Map class
				map_version, map_data = nil, nil
				map_header = {}
				File.open(dream_path, 'rb') do |f|
					map_version = f.gets.chomp

					while (line = f.gets.chomp) != 'BODY'
						key, value = line.split('=', 2)
						map_header[key] = value
					end

					map_data = f.read
				end

				@current_map_version = map_version
				@current_map_header = map_header

				width = map_header['width'].to_i
				height = map_header['height'].to_i
				@context.load_map map_data, width, height

				puts "Map data loaded (#{width}x#{height})"

				puts "Dream initialised."
			end

			def produce_map
				# TODO: Abstract this into a Map class
				@current_map_version + "\n" + @current_map_header.each_pair.map{|k,v| "#{k}=#{v}\n"}.join('') + "BODY\n" + @context.save_map
			end

			def load_ds_file(path)
				puts "Loading DS file..."

				tokens = File.open(path, 'r') do |f|
					Nelumbo::Script::Tokenizer.new(f).each_token.to_a
				end

				puts "(1/4) Tokenizer complete"

				line_parser = Nelumbo::Script::LineParser.new(Nelumbo::Script::DragonSpeak, tokens)
				lines = line_parser.lines.to_a

				puts "(2/4) Line parser complete"

				tree_parser = Nelumbo::Script::TreeParser.new(Nelumbo::Script::DragonSpeak)
				tree_parser.parse(lines)

				puts "(3/4) Tree parser complete"

				@ds_variables = tree_parser.variables_by_name

				# This code is messy and I should rework it some day..

				lines.each do |line|
					next unless line[:category]

					category = line.delete(:category)
					type = line.delete(:type)
					annotation = line.delete(:annotation)
					line.delete :number

					params = []

					spec = Nelumbo::Script::DragonSpeak.spec_for_line(category, type)

					# Note: Strings and string variables are ignored, and
					# simply passed through as 0. They are not necessary for
					# a DS engine which only needs the client-side stuff.
					line.each_value.zip(spec.each_value) do |token, spec_bit|
						if Array === token
							token.each do |tok|
								params << preprocess_ds_token(tok, spec_bit, tree_parser)
							end
						else
							params << preprocess_ds_token(token, spec_bit, tree_parser)
						end
					end

					#p category, type, params
					
					if annotation
						split_an = annotation.split
						annotation = {action: split_an.first.to_sym}
						
						case annotation[:action]
						when :event
							annotation[:name] = split_an[1]
						else
							puts "unknown annotation type: #{annotation[:action]}"
						end
					end

					@context.add_ds_line(category, type, params, annotation)
				end

				puts "(4/4) DS loaded"
			end

			def preprocess_ds_token(token, spec_bit, tree_parser)
				case token[:type]
				when :number
					token[:number]
				when :variable
					base = (spec_bit =~ /variable/) ? 0 : 50000
					base + tree_parser.index_of_variable(token)
				when :variable_pointer
					base = (spec_bit =~ /variable/) ? 0 : 50000
					base + token[:number]
				else
					0
				end
			end


			def ds_var(name)
				@context.variable(@ds_variables[name])
			end

			def ds_var_y(name)
				@context.variable(@ds_variables[name] + 1)
			end

			def set_ds_var(name, value)
				@context.set_variable(@ds_variables[name], value)
			end

			def set_ds_var_y(name, value)
				@context.set_variable(@ds_variables[name] + 1, value)
			end
		end


		module ClassMethods
			def setup_world_tracking
				on_enter_dream { reset_world_tracking }

				on_raw line: /^</ do
					uid,x,y,shape,name,colors,flags,afk = data[:line].furc_unpack('xDBBBSkAD')

					player = find_player_by_uid(uid) || create_and_add_player(uid, name)

					move_tracked_player player, x * 2, y
					player.shape = shape
					player.color_code = colors
					player.visible = (shape > 0)

					old_afk_time = player.afk_start_time
					was_previously_afk = player.afk?
					player.afk_start_time = (afk == 0) ? nil : (Time.now - afk)

					is_new = ((flags & 4) != 0)

					if is_new
						dispatch_event :player_entered, shortname: player.shortname,
							player: player, uid: uid, flags: flags
					else
						dispatch_event :player_seen, shortname: player.shortname,
							player: player, uid: uid, flags: flags
					end

					if was_previously_afk and afk == 0
						dispatch_event :player_unafk, player: player, duration: Time.now - old_afk_time
					elsif afk != 0 and !was_previously_afk
						dispatch_event :player_afk, player: player
					end
				end

				on_raw line: /^(\/|A)/ do
					uid,x,y,shape = data[:line].furc_unpack('xDBBB')

					player = request_player_by_uid(uid)
					move_tracked_player player, x * 2, y
					player.shape = shape
				end

				on_raw line: /^B/ do
					uid,shape,colors = data[:line].furc_unpack('xDBk')

					player = request_player_by_uid(uid)
					player.shape = shape
					player.color_code = colors
				end

				on_raw line: /^C/ do
					uid,x,y = data[:line].furc_unpack('xDBB')

					player = request_player_by_uid(uid)
					move_tracked_player player, x * 2, y
					player.visible = false
				end

				on_raw line: /^D/ do
					uid,x,y,shape,entry_code,held_object,cookies = data[:line].furc_unpack('xDBBBDDB')

					player = request_player_by_uid(uid)
					move_tracked_player player, x * 2, y
					player.shape = shape
					player.entry_code = entry_code
					player.held_object = held_object
					player.cookies = cookies
				end

				on_raw line: /^\)/ do
					uid = data[:line].furc_unpack('xD')

					player = find_player_by_uid(uid)
					halt if player.nil?

					delete_and_remove_player(player)

					dispatch_event :player_left, shortname: player.shortname, player: player, uid: uid
				end
			end
		end


		# Returns a Player matching either a user ID (Numeric), name
		# (String) or position (Numeric, Numeric). Pretty much just a
		# convenience function.
		def find_player(one, two=nil)
			return find_player_at_position(one, two) unless two.nil?
			return find_player_by_name(one) if String === one
			return find_player_by_uid(one)
		end

		# Identical to find_player, but halts the event if the user cannot
		# be found.
		def find_player!(one, two=nil)
			p = find_player(one, two)
			halt if p.nil?
			p
		end

		# Returns a Player matching the specified user ID.
		# If it isn't known to the bot, it requests the server to resend it
		# and halts the current event.
		def request_player_by_uid(uid)
			player = find_player_by_uid(uid)
			return player unless player.nil?

			write_line "rev #{uid.encode_b220(4)}"
			halt
		end


		def self.included(klass)
			klass.extend ClassMethods

			if Nelumbo.const_defined?(:World) and Nelumbo::World.const_defined?(:Context)
				klass.send :include, AdvancedImplementation
				klass.extend AdvancedImplementation::ClassMethods
			else
				klass.send :include, SimpleImplementation
				klass.extend SimpleImplementation::ClassMethods
			end

			klass.setup_world_tracking
		end
	end
end
