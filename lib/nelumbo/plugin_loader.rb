module Nelumbo
	# This is an optional module which can load and unload plugins dynamically
	# from Ruby source files.
	#
	# == Setup and Usage
	# Assign load_path to specify a directory for plugin files. Some other
	# properties may be changed.
	#
	# Call PluginLoader#load and PluginLoader#unload to do stuff.
	#
	# == Caveats
	# - Plugins must be modules named using a capitalised version of the
	#   filename with 'Plugin' added on the end. For example,
	#   +database_access.rb+ would contain +DatabaseAccessPlugin+.
	# - Unloading a plugin will delete its module (and any modules that are
	#   defined under it) but will make no other changes. It's not really
	#   feasible in Ruby to detect *everything* that a .rb file has touched.
	# - Don't load a plugin if another version with the same name is already
	#   loaded. No one stops you from doing that, but it's confusing.
	#
	module PluginLoader
		@plugins = []

		class << self
			attr_accessor :load_path

			def load(name)
				# normalise it
				file_name = File.join("#{@load_path}", "#{name.underscore}.rb")
				module_name = name.camelize + "Plugin"

				Kernel.load file_name

				mod = module_name.constantize
				
				@plugins << {mod: mod, name: module_name}
				mod
			end

			def unload(obj)
				plugin = nil

				if obj.is_a?(String)
					camel_name = obj.camelize
					plugin = @plugins.find{ |p| p[:name] == camel_name }
					camel_name += "Plugin"
					plugin ||= @plugins.find{ |p| p[:name] == camel_name }
				end

				if obj.is_a?(Module)
					plugin = @plugins.find{ |p| p[:mod] = obj }
				end

				raise "cannot find the loaded plugin matching #{obj}" if plugin.nil?

				@plugins.delete plugin

				mod = plugin[:mod]
				mod.parent.send :remove_const, mod.name.to_sym
				mod
			end
		end
	end
end
