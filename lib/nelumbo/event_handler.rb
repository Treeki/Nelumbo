module Nelumbo
	# The Nelumbo::EventHandler class provides a system for handling events using
	# a simple Sinatra-style DSL.
	#
	# EventHandler allows subclasses (and modules that extend EventDSL) to add
	# event responders using class methods, like this:
	#   class BaconBot < SomeBot
	#     on_connect { puts "We connected!" }
	#     on_message { puts "A message was received!" }
	#     on_message(text: /chunky bacon/i) { puts "Someone seems hungry..." }
	#     on_message(user: 'Treeki') do |data|
	#       puts "Treeki said #{data[:text]}!"
	#       halt_all if data[:text] =~ /do nothing else/i
	#     end
	#   end
	#
	# Events can be raised like this:
	#   bb = BaconBot.new
	#   bb.dispatch_event :connect
	#   bb.dispatch_event :message, user: 'Treeki', text: 'Chunky bacon!!'
	#   bb.dispatch_event :message, :user => 'Treeki', :text => 'alternate syntax is fun'
	#
	# Dispatching an event will send it to everything listed in
	# +singleton_class.ancestors+. The list is processed in reverse order, so
	# that base classes handle events first. There is one exception - the bot
	# object's class will ALWAYS be processed last. (Modules mixed in using
	# Mixology appear in the ancestors list before the top class for some
	# reason.)
	#
	class EventHandler
		extend Nelumbo::EventDSL
		setup_events
		
		# Return the data for the current event.
		def data
			@current_event_data
		end

		# Call the responders associated with an event.
		# An optional data hash can be passed containing information about the event.
		def dispatch_event(name, event_data = nil)
			# save the previous data so that events can be stacked
			saved_event_data = @current_event_data
			@current_event_data = event_data

			catch(:halt_all_responders) do
				# this will take care of all plugins (mixed-in modules)
				# AND subclasses. yay!
				singleton_class.ancestors.reverse_each do |mod|
					if mod != self.class and mod.respond_to?(:events)
						_exec_event_list(mod.events[name])
					end
				end

				# however, Mixology's got one little quirk: a mixed-in module
				# (in this case, a plugin) appears *above* the class in the
				# tree, so we ignore it in the loop above and process it here
				_exec_event_list(self.class.events[name])
			end

			@current_event_data = saved_event_data
		end

		# @private
		def _exec_event_list(event_list)
			return if event_list.nil?

			event_list.each do |responder|
				if _check_event_condition(responder[:conditions], @current_event_data)
					catch(:halt_this_responder) { instance_exec(&responder[:block]) }
				end
			end
		end

		# @private
		def _check_event_condition(conditions, event_data)
			return true if conditions.nil?
			return false if event_data.nil?

			conditions.all? { |k,v| v === event_data[k] }
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
