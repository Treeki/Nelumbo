module Nelumbo
	# Nelumbo::PlayerTracking is a module which can be included into a bot.
	# It will track players entering/leaving the dream and store their info.
	module PlayerTracking
		class Player
			def initialize(uid, name)
				@uid = uid
				@name = name
				@shortname = name.to_shortname
			end

			attr_reader :uid, :shortname
			attr_accessor :name, :color_code
			attr_accessor :x, :y, :shape, :visible, :afk_time
		end

		module ClassMethods
			def setup_player_tracking
				define_event_with_args :player_entered
				define_event_with_args :player_left

				on_init_bot do
					@player_list = []
					@player_lookup_by_uid = {}
					@player_lookup_by_shortname = {}
				end

				on_raw line: /^</ do
					uid,x,y,shape,name,colors,flags,afk = data[:line].furc_unpack('xDBBBS!AD')

					player = Player.new(uid, name)
					player.x = x
					player.y = y
					player.shape = shape
					player.color_code = colors
					player.visible = ((flags & 2) != 0)
					player.afk_time = afk

					@player_list << player
					@player_lookup_by_uid[uid] = player
					@player_lookup_by_shortname[player.shortname] = player

					dispatch_event :player_entered,
						player: player, uid: uid, shortname: player.shortname,
						is_new: ((flags & 4) != 0)
				end

				on_raw line: /^(\/|A)/ do
					uid,x,y,shape = data[:line].furc_unpack('xDBBB')

					player = request_player_by_uid(uid)
					player.x = x
					player.y = y
					player.shape = shape
				end

				on_raw line: /^B/ do
					uid,shape,colors = data[:line].furc_unpack('xDB!')

					player = request_player_by_uid(uid)
					player.shape = shape
					player.color_code = colors
				end

				on_raw line: /^C/ do
					uid,x,y = data[:line].furc_unpack('xDBB')

					player = request_player_by_uid(uid)
					player.x = x
					player.y = y
					player.visible = false
				end

				on_raw line: /^D/ do
					uid,x,y,shape,entry_code,held_object,cookies = data[:line].furc_unpack('xDBBBDDB')

					player = request_player_by_uid(uid)
					player.x = x
					player.y = y
					player.shape = shape
					player.entry_code = entry_code
					player.held_object = held_object
					player.cookies = cookies
				end

				on_raw line: /^\)/ do
					uid = data[:line].furc_unpack('xD')

					player = find_player_by_uid(uid)
					halt if player.nil?

					@player_list.delete player
					@player_lookup_by_uid.delete uid
					@player_lookup_by_shortname.delete player.shortname

					dispatch_event :player_left, player: player, uid: uid, shortname: player.shortname
				end
			end
		end

		def self.included(klass)
			klass.extend ClassMethods
			klass.setup_player_tracking
		end

		# Returns a Nelumbo::Player matching the specified user ID
		def find_player_by_uid(uid)
			@player_lookup_by_uid[uid]
		end

		# Returns a Nelumbo::Player matching the specified name, taking
		# shortnames/longnames into account
		def find_player_by_name(name)
			@player_lookup_by_shortname[name.to_shortname]
		end

		# If a block is given, each player known to the bot is passed to it.
		# Otherwise, an enumerator is returned.
		def each_player
			if block_given?
				@player_list.each {|player| yield player}
			else
				@player_list.each
			end
		end


		def request_player_by_uid(uid)
			player = find_player_by_uid(uid)
			write_line "rev #{uid.encode_b220}"
		end
	end
end
