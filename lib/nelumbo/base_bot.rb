module Nelumbo
	# The Nelumbo::BaseBot class implements a simple bot that can communicate
	# with Furcadia, handle events using Nelumbo::EventHandler and have
	# plugins loaded.
	#
	# Socket and timer handling is done using a Core, which can be passed to
	# BaseBot#new. If none is specified, a new instance of Nelumbo::SimpleCore
	# will be used.
	#
	# Usage of this class directly is not recommended. Nelumbo::Bot implements
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
	class BaseBot < EventHandler
		include CoreHooks

		attr_reader :core, :state

		def initialize(core = nil)
			@core = (core || SimpleCore.new)
			@state = :inactive
			@plugins = []
			@timers = []
			@recurring_timers = []
			collect_recurring_timers
		end

		# Connect and run the bot. This method will block until the bot disconnects.
		# Note: If you are not using Nelumbo::SimpleCore (the default when no
		# core is specified), then this method may do nothing.
		def run
			@core.run(self)
		end

		# Set an action to occur once after a specific amount of time has passed.
		# This method can trigger either an event or a block.
		#   after(30, :event_name)
		#   after(30) { puts "30 seconds passed" }
		#
		def after(duration, event=nil)
			info = {trigger_at: Time.now + duration}
			if event
				info[:event] = event
			else
				raise "block not passed to BaseBot#after" unless block_given?
				info[:block] = Proc.new
				info[:event_data] = data
			end

			@timers << info
		end

		# Hook called by the Core at a specific interval.
		# The time between ticks is not fixed.
		def timer_tick
			current_time = Time.now

			complete = nil

			@timers.each do |info|
				next if info[:trigger_at] > current_time

				if info[:block]
					with_event_data(info[:event_data]) do
						instance_exec &info[:block]
					end
				else
					dispatch_event info[:event]
				end
				(complete ||= []) << info
			end

			@timers -= complete if complete

			# now process recurring timers
			@recurring_timers.each do |info|
				next if info[:trigger_at] > current_time

				instance_exec &info[:block]
				info[:trigger_at] += info[:interval]
			end
		end

		# Grab every recurring timer from the class and module and store it
		# into an instance variable. This method must be called if they are
		# modified. (Adding/removing plugins will automatically call it. It is
		# also called when the class is created.)
		#
		def collect_recurring_timers
			# there's a bit of a dilemma here: We have to regenerate the list,
			# but we don't want to lose the existing time values.
			#
			# The hash containing the original info (held by the class/module)
			# doesn't change, so we store it in info[:base]. This can be used
			# to determine which ones stayed the same.

			to_keep = {}
			@recurring_timers.each{ |t| to_keep[t[:base]] = t[:trigger_at] }

			@recurring_timers = []
			singleton_class.ancestors.reverse_each do |mod|
				if mod.respond_to?(:recurring_timers)
					mod.recurring_timers.each do |t|
						# create a timer and calculate the next trigger time
						info = {base: t, block: t[:block], interval: t[:interval]}
						info[:trigger_at] = to_keep[t] || (Time.now + t[:interval])

						@recurring_timers << info
					end
				end
			end
		end

		# Hook called by the Core when a line is received from the server.
		def line_received(line)
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

		# Hook called by the Core when the bot is about to start.
		def bot_started
			dispatch_event :init_bot
			@state = :login
		end

		# Hook called by the Core when the bot is done running.
		def bot_ended
			dispatch_event :disconnect
			@state = :inactive

			plugins.each do |plugin|
				remove_plugin plugin
			end
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
			collect_recurring_timers
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
			collect_recurring_timers
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
