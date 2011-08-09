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
	#   Data: +:name+, +:shortname+, +:text+
	# [whisper]
	#   A player whispered the bot.
	#   Data: +:name+, +:shortname+, +:text+
	#
	class BaseBot < EventHandler
		include CoreHooks

		attr_reader :core, :state

		def initialize(core = nil)
			@core = (core || SimpleCore.new)
			@state = :inactive
		end

		# Connect and run the bot. This method will block until the bot disconnects.
		# Note: If you are not using Nelumbo::SimpleCore (the default when no
		# core is specified), then this method may do nothing.
		def run
			dispatch_event :init_bot
			@state = :login
			@core.run(self)
		end


		# Hook called by the Core at a specific interval.
		# The time between ticks is not fixed.
		def timer_tick
			# TODO: handle this
			#puts 'timer ticked'
		end

		# Hook called by the Core when a line is received from the server.
		def line_received(line)
			return if @state == :login and try_parse_login(line)

			p line

			dispatch_event :raw, line: line

			if line[0] == '('
				dispatch_event :message, line: line.from(1)

				try_parse_speech(line)
				return
			end

			if line[0,2] == ']c'
				dispatch_event :enter_dream
			end
		end


		def load_plugin(mod)
			mixin mod
			mod.plugin_loaded(self)
		end

		def unload_plugin(mod)
			mod.plugin_unloaded(self)
			unmix mod
		end


		def say(line)
			write_line "\"#{line}"
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
				dispatch_event :speech, name: name, shortname: shortname, text: message
			elsif /^\(<font color='whisper'>\[ <name shortname='(?<shortname>[^']+)' src='whisper-from'>(?<name>[^<]+)<\/name> whispers, "(?<message>.+)" to you. \]<\/font>$/ =~ line
				dispatch_event :whisper, name: name, shortname: shortname, text: message
			else
				return false
			end

			true
		end
	end
end
