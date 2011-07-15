module Nelumbo
	# A module that provides the standard Furcadia events to any
	# EventHandler subclass.
	#   include Nelumbo::FurcEvents
	#   setup_furc_events
	#
	module FurcEvents
		module ClassMethods
			def setup_furc_events
				# Connection Housekeeping
				define_event :init_bot
				define_event :connect
				define_event :login
				define_event :disconnect

				# Basic
				define_event_with_args :raw
				define_event_with_args :message

				# Interaction
				define_event_with_args :speech
				define_event_with_args :emote
				define_event_with_args :whisper
				define_event_with_args :emit

				# Dreams
				define_event :enter_dream
				define_event :ejected
			end
		end

		def self.included(klass)
			klass.extend ClassMethods
			klass.setup_furc_events
		end
	end
end

