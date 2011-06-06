module Nelumbo
	# Provides the hooks required for a Bot to communicate with a Core.
	module CoreHooks
		def set_core_hooks(write_line, disconnect)
			@core_hook_write_line = write_line
			@core_hook_disconnect = disconnect
		end

		def write_line(line)
			@core_hook_write_line.call(line)
		end

		def disconnect
			@core_hook_disconnect.call
		end
	end
end

