require 'active_support/all'

require 'mixology'

require 'socket'
require 'set'

# C extension
require 'nelumbo/nelumbo'

# Submodules
require 'nelumbo/core_ext'
require 'nelumbo/script'

# Main stuff (TODO: Move into a Bot module)
require 'nelumbo/event_dsl'
require 'nelumbo/event_handler'
require 'nelumbo/plugin'
require 'nelumbo/core_hooks'
require 'nelumbo/select_core'
require 'nelumbo/simple_core'
require 'nelumbo/base_bot'
require 'nelumbo/bot'
require 'nelumbo/world_tracking'

module Nelumbo
	VERSION = '0.0.1'
end
