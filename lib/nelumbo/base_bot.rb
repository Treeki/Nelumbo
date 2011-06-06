module Nelumbo
	# The Nelumbo::BaseBot class implements a simple bot that can communicate
	# with Furcadia and handle events using Nelumbo::EventHandler.
	#
	# Socket and timer handling is done using a Core, which can be passed to
	# BaseBot#new. If none is specified, a new instance of Nelumbo::SimpleCore
	# will be used.
	#
	# Usage of this class directly is not recommended. Nelumbo::Bot implements
	# plugins and various other features.
	#
	class BaseBot < EventHandler
		include FurcEvents
		include CoreHooks

		attr_reader :core

		def initialize(core = nil)
			@core = (core || SimpleCore.new)
		end

		# Connect and run the bot. This method will block until the bot disconnects.
		# Note: If you are not using Nelumbo::SimpleCore (the default when no
		# core is specified), then this method may do nothing.
		def run
			@core.run(self)
		end


		# Hook called by the Core at a specific interval.
		# The time between ticks is not fixed.
		def timer_tick
			# TODO: handle this
			puts 'timer ticked'
		end

		# Hook called by the Core when a line is received from the server.
		def line_received(line)
			puts line

			return if try_parse_login(line)
			return if try_parse_speech(line)
		end

		private
		def try_parse_login(line)
			case line
			when 'Dragonroar'
				dispatch_event :connect
			when '&&&&&&&&&&&&&'
				dispatch_event :login
			else
				return false
			end

			true
		end

		def try_parse_speech(line)
			if /^\(<font color='whisper'>\[ <name shortname='(?<shortname>[^']+)' src='whisper-from'>(?<name>[^<]+)<\/name> whispers, "(?<message>.+)" to you. \]<\/font>$/ =~ line
				dispatch_event :whisper, name: name, shortname: shortname, text: message
			else
				return false
			end

			true
		end
	end
end
