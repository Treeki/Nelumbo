module Nelumbo
	module Plugin
		extend Nelumbo::EventDSL

		def self.plugin_loaded(bot)
			bot.instance_exec(@plugin_load_hook) unless @plugin_load_hook.nil?
		end

		def self.plugin_unloaded(bot)
			bot.instance_exec(@plugin_unload_hook) unless @plugin_unload_hook.nil?
		end

		def self.when_loaded(&block)
			@plugin_load_hook = block
		end

		def self.when_unloaded(&block)
			@plugin_unload_hook = block
		end
	end
end
