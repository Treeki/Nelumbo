module Nelumbo
	# This module can be included into any other module to turn it into a
	# Nelumbo plugin. Events can be defined as usual within the module.
	#
	# A specific bot's plugin modules can be managed using BaseBot#add_plugin
	# and BaseBot#remove_plugin.
	#
	# The +when_added+ and +when_removed+ methods will execute a block when
	# the plugin is added or removed to/from a bot.
	#   when_added { puts "The plugin was added to #{self}!" }
	#   when_removed { puts "The plugin was removed from #{self}!" }
	#
	# == Related
	# If you need to use it, the Nelumbo::PluginLoader singleton class will
	# handle loading/unloading of plugins from Ruby code files, with a couple
	# of caveats. See the class's documentation for more.
	#
	module Plugin
		def self.included(mod)
			mod.extend ClassMethods
			mod.extend Nelumbo::EventDSL
			mod.setup_events
			mod.setup_as_plugin
		end

		module ClassMethods
			def setup_as_plugin
				@plugin_instances = []
			end

			def plugin_added(bot)
				@plugin_instances << bot
				bot.instance_exec(&@plugin_add_hook) unless @plugin_add_hook.nil?
			end

			def plugin_removed(bot)
				@plugin_instances.delete bot
				bot.instance_exec(&@plugin_remove_hook) unless @plugin_remove_hook.nil?
			end

			def users
				@plugin_instances
			end

			def when_added(&block)
				@plugin_add_hook = block
			end

			def when_removed(&block)
				@plugin_remove_hook = block
			end
		end
	end
end
