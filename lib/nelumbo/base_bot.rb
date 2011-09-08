module Nelumbo
	# The Nelumbo::BaseBot class implements a simple bot that can communicate
	# with Furcadia, handle events using Nelumbo::EventHandler and have
	# plugins loaded.
	#
	# Do not instantiate this class directly. See Nelumbo::EventHandler's docs
	# for details. To instantiate a bot, use Nelumbo::start.
	#
	# Subclassing this class directly is not recommended. Nelumbo::Bot implements
	# login and various other niceties that are useful for most bots.
	#
	# BaseBot will raise these events:
	# [init_bot]
	#   Raised right before the bot connects.
	# [connect]
	#   Raised when the bot has connected and needs to log in.
	# [login]
	#   Raised when the bot has logged in.
	# [disconnect]
	#   Raised when the bot has disconnected.
	# [raw]
	#   A line is received from the server.
	#   Data: +:line+
	# [message]
	#   A visible message is received from the server.
	#   Data: +:line+
	# [speech]
	#   A player spoke within the bot's range.
	#   Data: +:text+, +:name+, +:shortname+
	# [whisper]
	#   A player whispered the bot.
	#   Data: +:text+, +:name+, +:shortname+
	# [ds_emit]
	#   A DragonSpeak emit was received, and it did not trigger the below
	#   event.
	#   Data: +:text+
	# [_unspecified_]
	#   A DragonSpeak emit was received that triggered an event. The emit must
	#   be formatted like this: +evt event_name optional arguments+
	#   Data: +:args+ (string containing everything after the event name)
	#
	class BaseBot < EM::Connection
		include EventHandler
		extend EventDSL
		setup_events

		attr_reader :state

		def initialize
			super

			@state = :inactive
			@plugins = []

			# Timers!
			@timers = []
			@recurring_timers = {}
			collect_initial_recurring_timers

			# Networking fun stuff
			@receive_buffer = ''
			@output_buffer = []
			@output_timer = EM::add_periodic_timer(0.1, method(:write_line_from_buffer))
		end

		# Set an action to occur once after a specific amount of time has passed.
		# This method can trigger either an event or a block.
		#   after(30, :event_name)
		#   after(30) { puts "30 seconds passed" }
		#
		def after(duration, event=nil)
			if event
				run = proc { dispatch_event event }
			else
				raise "block not passed to BaseBot#after" unless block_given?

				saved_data = data
				block = Proc.new
				run = proc {
					with_event_data(saved_data, &block)
				}
			end

			@timers << EM::add_timer(duration, run)
		end

		# @private
		# Grab every recurring timer from the class and its ancestors (but
		# NOT from dynamically loaded modules) and keep track of them.
		#
		def collect_initial_recurring_timers
			@recurring_timers = {}
			singleton_class.ancestors.reverse_each do |mod|
				if mod.respond_to?(:recurring_timers)
					list = @recurring_timers[mod] = []

					mod.recurring_timers.each do |t|
						list << EM::add_periodic_timer(t[:interval], &t[:block])
					end
				end
			end
		end

		# @private
		# Remove every timer known to the bot right now.
		#
		def remove_all_recurring_timers
			@recurring_timers.each_value do |list|
				list.each { |timer| EM::cancel_timer(timer) }
			end
		end

		# @private
		# Add every recurring timer contained within a module.
		#
		def add_timers_for(mod)
			list = @recurring_timers[mod] = []

			mod.recurring_timers.each do |t|
				list << EM::add_periodic_timer(t[:interval], &t[:block])
			end
		end

		# @private
		# Remove and cancel every recurring timer for a module.
		#
		def remove_timers_for(mod)
			list = @recurring_timers.delete(mod)
			list.each { |timer| EM::cancel_timer(timer) }
		end

		# Hook called by EventMachine. Do not call this function directly.
		def receive_data(data)
			packet = (@receive_buffer + data).split("\n")

			if data.end_with?("\n")
				@receive_buffer = ''
			else
				@receive_buffer = packet.pop
			end

			for line in packet
				line_received(line)
			end
		end

		# @private
		# Process a line.
		#
		def line_received(line)
			return if line.empty?
			return if @state == :login and try_parse_login(line)

			#p line

			dispatch_event :raw, line: line

			if line.start_with?(?()
				try_parse_speech(line) or dispatch_event :message, line: line.from(1)
				return
			end

			if line.start_with?(']c')
				dispatch_event :enter_dream
			end
		end

		# Hook called by EventMachine when the bot has connected.
		def connection_completed
			dispatch_event :init_bot
			@state = :login
		end

		# Hook called by EventMachine when the bot is done.
		def unbind
			dispatch_event :disconnect
			@state = :inactive

			plugins.each do |plugin|
				remove_plugin plugin
			end

			remove_all_recurring_timers

			@output_timer.cancel

			@bot_disconnected_hook.call if @bot_disconnected_hook
		end

		# Sets a block that will be called when the bot disconnects.
		# Currently used by Nelumbo::run_simply.
		#
		def when_disconnected(&block)
			@bot_disconnected_hook = block
		end



		# Write a line to the bot. Line terminators are not required.
		def write_line(line)
			@output_buffer << line
		end

		# @private
		def write_line_from_buffer
			if (line = @output_buffer.shift)
				puts "Sending:", line.inspect
				send_data "#{line}\n"
			end
		end

		# Finish up! Disconnects the bot.
		def disconnect
			close_connection
		end



		def plugin?(mod)
			@plugins.include?(mod)
		end

		attr_reader :plugins

		# Adds a plugin module to the current bot.
		#
		# Returns false if that module is already in this bot.
		# If it was successfully added, the module is returned.
		#
		def add_plugin(mod)
			return false if plugin?(mod)

			mixin mod
			mod.plugin_added(self)
			add_timers_for(mod)
			@plugins << mod
			mod
		end

		# Removes a plugin module from the current bot.
		#
		# Returns false if that module is not in this bot.
		# If it was successfully removed, the module is returned.
		#
		def remove_plugin(mod)
			return false unless plugin?(mod)

			@plugins.delete mod
			mod.plugin_removed(self)
			unmix mod
			remove_timers_for(mod)
			mod
		end

		# Loads and adds a plugin to this bot using Nelumbo::PluginLoader.
		#
		# Returns false if PluginLoader already has this plugin loaded.
		# Otherwise, the return value is the same as add_plugin.
		#
		def load_plugin(name)
			return false if PluginLoader.known?(name)

			mod = PluginLoader.load(name)
			add_plugin mod
		end

		# Removes a plugin from this bot and unloads it using
		# Nelumbo::PluginLoader.
		#
		# Returns false if PluginLoader does not know about this plugin.
		# Otherwise, the return value is the same as remove_plugin.
		#
		def unload_plugin(name)
			return false unless PluginLoader.known?(name)

			mod = PluginLoader.module_for(name)
			remove_plugin mod
			PluginLoader.unload(mod)
			mod
		end


		def say(line)
			write_line "\"#{line}"
		end

		# This method says something but tries to avoid triggering any DS
		# commands by using an emote starting with ::. Any user-controlled
		# input should be passed through this method when spoken - you never
		# know what may be lurking in that seemingly-innocuous string!
		def speak(line)
			write_line ":: #{line}"
		end

		def emote(line)
			write_line ":#{line}"
		end

		def whisper(name, text)
			write_line "wh #{name.gsub(' ', '|')} #{text}"
		end

		def whisper_back(text)
			write_line "wh #{data[:shortname]} #{text}"
		end

		private
		def try_parse_login(line)
			case line
			when 'Dragonroar'
				dispatch_event :connect
			when /^&/
				@state = :active
				dispatch_event :login
			else
				return false
			end

			true
		end

		def try_parse_speech(line)
			if /^\(<name shortname='(?<shortname>[^']+)'>(?<name>[^<]+)<\/name>: (?<message>.+)$/ =~ line
				dispatch_event :speech, text: message, name: name, shortname: shortname

			elsif /^\(<font color='whisper'>\[ <name shortname='(?<shortname>[^']+)' src='whisper-from'>(?<name>[^<]+)<\/name> whispers, "(?<message>.+)" to you. \]<\/font>$/ =~ line
				dispatch_event :whisper, text: message, name: name, shortname: shortname

			elsif /^\(<font color='dragonspeak'><img src='fsh:\/\/system\.fsh:91' alt='@emit' \/><channel name='@emit' \/> (?<message>.+)<\/font>$/ =~ line
				# DragonSpeak emit
				if message.start_with?('evt ')
					cmd, event, args = message.split(' ', 3)
					dispatch_event event.to_sym, args: args
				else
					dispatch_event :ds_emit, text: message
				end

			else
				return false
			end

			true
		end
	end
end
