module Nelumbo
	# The Nelumbo::EventHandler class provides a system for handling events using
	# a simple Sinatra-style DSL.
	#
	# A subclass of EventHandler can define various events (with optional conditions).
	#   class SomeBot < Nelumbo::EventHandler
	#     define_event :connect
	#     define_event_with_args :message
	#     define_event(:random) { rand(3) == 1 } # this is not a bad idea. really!
	#   end
	#
	# This class can in turn be subclassed by a user, who can add event responders
	# using class methods like this:
	#   class BaconBot < SomeBot
	#     on_connect { puts "We connected!" }
	#     on_message { puts "A message was received!" }
	#     on_message(text: /chunky bacon/i) { puts "Someone seems hungry..." }
	#     on_message(user: 'Treeki') do |data|
	#       puts "Treeki said #{data[:text]}!"
	#       halt_all if data[:text] =~ /do nothing else/i
	#     end
	#     on_random { puts "This might or might not happen!" }
	#   end
	#
	# Events can be called like this:
	#   bb = BaconBot.new
	#   bb.dispatch_event :connect
	#   bb.dispatch_event :message, user: 'Treeki', text: 'Chunky bacon!!'
	#   bb.dispatch_event :message, :user => 'Treeki', :text => 'alternate syntax is fun'
	#
	class EventHandler
		class << self
			attr_reader :events

			# Initialise the events array for the class.
			def setup_events
				@events = {}

				# copy the events that exist in every base class
				steal_event_definitions(superclass)
			end

			# Hook that calls setup_events on every subclass of EventHandler,
			# so that each one can have its own @events hash and avoid conflicts.
			def inherited(cls)
				cls.setup_events
			end

			# Hook that allows a subclass to define an event responder using on_event_name.
			def method_missing(name, *args, &block)
				return super unless /^on_(?<event_name>.+)$/ =~ name.to_s

				create_event_responder event_name.to_sym, args.first, &block
			end

			private
			# Create a definition for each event that exists in the ancestors of
			# this class. Event responders themselves are not copied.
			def steal_event_definitions(cls)
				return unless cls.respond_to?(:events)

				steal_event_definitions(cls.superclass)

				to_add = cls.events.keys - @events.keys
				to_add.each { |name| define_event(name, &cls.events[name][:check_block]) }
			end

			# Define a new event type.
			# If a block is given, then it is used to determine whether a responder
			# should be called or not. It is passed the responder's condition hash and
			# the event data hash.
			def define_event(name, &block)
				raise "event #{name} is already defined" if @events.include?(name)

				block = proc{true} unless block_given?
				@events[name] = {:check_block => block, :responders => []}
			end

			# Define a new event type which uses arguments. No block can be given.
			# It uses the conditions specified by the responder and checks them against
			# the event data hash using the === operator.
			def define_event_with_args(name)
				define_event(name) do |conditions,event_data|
					conditions.nil? or conditions.all?{|k,v| v === event_data[k]}
				end
			end

			# Add a new event responder.
			def create_event_responder(name, conditions, &block)
				raise "unknown event #{name}" unless @events.include? name
				raise "block not provided for #{name}" unless block_given?
				@events[name][:responders] << {:conditions => conditions, :block => block}
			end
		end

		setup_events

		
		# Return the data for the current event.
		def data
			@current_event_data
		end

		# Call the responders associated with an event.
		# An optional data hash can be passed containing information about the event.
		def dispatch_event(name, event_data = nil, event_cls = nil)
			# this bit is inspired by Sinatra
			event_cls ||= self.class
			dispatch_event(name, event_data, event_cls.superclass) if event_cls.superclass.respond_to?(:events)

			event = event_cls.events[name]
			return if event.nil?
			# TODO: must come up with a better method for this which doesn't choke
			# on subclassing
			#raise "cannot dispatch unknown event #{name}" if event.nil?
			checker = event[:check_block]

			@current_event_data = event_data

			catch(:halt_all_responders) do
				event[:responders].each do |responder|
					if instance_exec(responder[:conditions], event_data, &checker)
						catch(:halt_this_responder) { instance_exec(&responder[:block]) }
					end
				end
			end
		end


		# Halt processing for the current responder.
		def halt
			throw :halt_this_responder
		end

		# Halt processing for the current event.
		def halt_all
			throw :halt_all_responders
		end
	end
end
