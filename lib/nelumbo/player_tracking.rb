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
			attr_accessor :x, :y, :visible, :afk_start_time

			# transient info that may or may not be up-to-date
			attr_accessor :entry_code, :shape, :held_object, :cookies
		end

		module ClassMethods
			def setup_player_tracking
				define_event_with_args :player_entered
				define_event_with_args :player_left

				on_init_bot do
					@player_list = []
					@player_lookup_by_uid = {}
					@player_lookup_by_shortname = {}
					@player_lookup_by_position = {}
				end

				on_raw line: /^</ do
					uid,x,y,shape,name,colors,flags,afk = data[:line].furc_unpack('xDBBBSkAD')

					player = find_player_by_uid(uid)
					if player.nil?
						player = Player.new(uid, name)

						@player_list << player
						@player_lookup_by_uid[uid] = player
						@player_lookup_by_shortname[player.shortname] = player
					end

					move_player player, x, y
					player.shape = shape
					player.color_code = colors
					player.visible = (shape > 0)
					player.afk_start_time = (afk > 0) ? nil : (Time.now - afk)

					dispatch_event :player_entered,
						player: player, uid: uid, shortname: player.shortname,
						is_new: ((flags & 4) != 0), flags: flags
				end

				on_raw line: /^(\/|A)/ do
					uid,x,y,shape = data[:line].furc_unpack('xDBBB')

					player = request_player_by_uid(uid)
					move_player player, x, y
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
					move_player player, x, y
					player.visible = false
				end

				on_raw line: /^D/ do
					uid,x,y,shape,entry_code,held_object,cookies = data[:line].furc_unpack('xDBBBDDB')

					player = request_player_by_uid(uid)
					move_player player, x, y
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
					@player_lookup_by_position.delete (player.x << 12) | player.y

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

		# Returns a Nelumbo::Player at the specified position
		def find_player_at_position(x, y)
			@player_lookup_by_position[(x << 12) | y]
		end

		# Moves a player to a position. This *MUST* be used or
		# find_player_at_position will not work correctly!
		def move_player(player, x, y)
			return if player.x == x and player.y == y

			@player_lookup_by_position.delete (x << 12) | y
			player.x = x
			player.y = y
			@player_lookup_by_position[(x << 12) | y] = player
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


		# Returns a Nelumbo::Player matching the specified user ID.
		# If it isn't known to the bot, it requests the server to resend it
		# and halts the current event.
		def request_player_by_uid(uid)
			player = find_player_by_uid(uid)
			return player unless player.nil?

			write_line "rev #{uid.encode_b220(4)}"
			halt
		end
	end
end
