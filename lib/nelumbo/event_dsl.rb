module Nelumbo
	# Internal module relating to the event system.
	# This is meant for extending a class/module, not being included.
	#
	# If you don't know what to do with it, you probably don't need to touch
	# it.
	#
	module EventDSL
		attr_reader :events

		# Initialise the events array for the class. Don't call this
		# method unless you know what you're doing!
		def setup_events
			@events = {}
		end

		# Hook that calls setup_events on every subclass of a class that
		# extends EventDSL, so that each one can have its own @events hash and
		# avoid conflicts.
		def inherited(cls)
			cls.setup_events
		end

		def included(mod)
			mod.setup_events
		end

		# Hook that allows a subclass to define an event responder using on_event_name.
		def method_missing(name, *args, &block)
			return super unless /^on_(?<event_name>.+)$/ =~ name.to_s

			create_event_responder event_name.to_sym, args.first, &block
		end

		private
		# Add a new event responder.
		def create_event_responder(name, conditions, &block)
			raise "block not provided for #{name}" unless block_given?
			(@events[name] ||= []) << {conditions: conditions, block: block}
		end
	end
end

