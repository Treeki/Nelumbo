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
require 'nelumbo/core_hooks'
require 'nelumbo/select_core'
require 'nelumbo/simple_core'
require 'nelumbo/base_bot'
require 'nelumbo/bot'
require 'nelumbo/world_tracking'

# Nelumbo is a flexible bot framework for the MMOSG Furcadia focusing on clean
# and short code, written by Treeki. See the README for more information.
module Nelumbo
	VERSION = '0.0.1'
end
