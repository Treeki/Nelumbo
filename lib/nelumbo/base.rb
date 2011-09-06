$eventmachine_library = :pure_ruby
require 'eventmachine'

require 'mixology'

require 'singleton'
require 'socket'
require 'set'

require 'active_support/inflector'
require 'active_support/core_ext/array/conversions'
require 'active_support/core_ext/array/access'
require 'active_support/core_ext/array/random_access'
require 'active_support/core_ext/float'
require 'active_support/core_ext/integer/inflections'
require 'active_support/core_ext/string/access'
require 'active_support/core_ext/string/filters'
require 'active_support/core_ext/string/inflections'
require 'active_support/core_ext/module/introspection'

# need to decide if I want the Date/Time stuff or not

# C extension
require 'nelumbo/nelumbo'

# Submodules
require 'nelumbo/core_ext'
require 'nelumbo/script'

# Main stuff (TODO: Move into a Bot module)
require 'nelumbo/event_dsl'
require 'nelumbo/event_handler'
require 'nelumbo/plugin'
require 'nelumbo/plugin_loader'
require 'nelumbo/base_bot'
require 'nelumbo/bot'
require 'nelumbo/world_tracking'

# Nelumbo is a flexible bot framework for the MMOSG Furcadia focusing on clean
# and short code, written by Treeki. See the README for more information.
module Nelumbo
	VERSION = '0.0.1'


	# Start an instance of a specified bot.
	def self.begin_bot(klass)
		EM::connect('lightbringer.furcadia.com', 6500, klass)
	end

	# Start an instance of this bot and run it in an EventMachine event loop.
	# The loop will automatically be terminated when this bot stops.
	def self.run_simply(klass)
		EventMachine::run {
			bot = begin_bot(klass)
			bot.when_disconnected { EM::stop_event_loop }
		}
	end
end
