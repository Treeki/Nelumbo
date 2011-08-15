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
		def params
			@current_event_data
		end
		alias :data :params

		def param
			@current_event_data.first[1]
		end

		# Temporarily override the event data while executing some code.
		def with_event_data(event_data)
			saved = @current_event_data
			@current_event_data = event_data
			yield
			@current_event_data = saved
		end

		# Call the responders associated with an event.
		# An optional data hash can be passed containing information about the event.
		#
		# The first element in the hash is assumed to be a "default" entry, and
		# will be treated as such when checking conditions. For example, these two
		# event definitions do the same thing when the following event is raised:
		#   dispatch_event(:speech, text: 'asdf', name: 'Cat', shortname: 'cat')
		#   on_speech('asdf') { puts "Event activated" }
		#   on_speech(text: 'asdf') { puts "Event activated" }
		#
		def dispatch_event(name, event_data = nil)
			# save the previous data so that events can be stacked
			saved_event_data = @current_event_data
			@current_event_data = event_data

			catch(:halt_all_responders) do
				# this will take care of all plugins (mixed-in modules)
				# AND subclasses. yay!
				singleton_class.ancestors.reverse_each do |mod|
					if mod != self.class and mod.respond_to?(:events)
						begin
							_exec_event_list(mod.events[name])
						rescue Exception => error
							# oops, something went wrong
							# raise :plugin_error if it's in a plugin
							# if not, then just throw the exception up the stack
							if name != :plugin_error and mod.include?(Nelumbo::Plugin)
								dispatch_event :plugin_error,
									plugin: mod, error: error
							else
								raise error
							end
						end
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

			default = conditions[:__default]
			(conditions.all? { |k,v| k == :__default or v === event_data[k] } and
			 (default.nil? or default === event_data.first[1]))
		end


		# Halt processing for the current responder.
		def halt
			throw :halt_this_responder
		end

		# Halt processing for the current event.
		def halt_all
			throw :halt_all_responders
		end


		# Hook that allows event data/params to be accessed using the
		# event_argname method (for example, event_name returns params[:name]).
		#
		# After the first usage of a method, it is created to avoid the
		# method_missing overhead.
		#
		def method_missing(name, *args, &block)
			return super unless /^event_(?<param_name>.+)$/ =~ name

			self.class_eval <<-END
				def event_#{param_name}
					params[:#{param_name}]
				end
			END

			params[param_name.to_sym]
		end
	end
end
