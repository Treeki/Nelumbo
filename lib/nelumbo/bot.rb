module Nelumbo
	# The Nelumbo::Bot class implements a general-purpose bot for Furcadia.
	#
	# == Settings Manager
	# Settings can be assigned to a class and inherited by subclasses of that
	# class, using the Bot::set method. They can be queried using Bot::setting
	# and Bot::setting?.
	#
	# Individual instances of a class can also have their own settings, which
	# can be modified at any time and override (but do not replace) the class
	# settings.
	#
	# == Connection
	# The Bot class handles logging in for you.
	#
	# The :username and :password settings are not optional:
	#   set username: 'Treeki', password: 'hunter2'
	# The :color_code and :description settings are optional, but recommended.
	# Alternatively, a :costume setting can be used.
	# Usage is fairly obvious.
	#
	class Bot < BaseBot
		class << self
			attr_reader :settings

			# Initialise settings for the class. Don't call this method unless
			# you know what you're doing!
			def setup_settings
				@settings = {}
			end

			def inherited(cls)
				super
				cls.setup_settings
			end

			# Get a specific setting for this class.
			# If the setting does not exist in this class, its ancestors will
			# be checked for the setting.
			def setting(name)
				return @settings[name] if @settings.include?(name)
				return superclass.setting(name) if superclass.respond_to?(:setting)
				nil
			end

			# Get whether a specific setting exists in this class or not.
			# If the setting does not exist in this class, its ancestors will
			# be checked.
			def setting?(name)
				return true if @settings.include?(name)
				return superclass.setting?(name) if superclass.respond_to?(:setting?)
				false
			end

			# Assign settings to this class from a hash.
			#   set username: 'Treeki', password: 'hunter2'
			def set(hash)
				@settings.update(hash)
			end
		end

		def initialize(*)
			super
			@settings = {}
		end

		# Get a specific setting in this object.
		# If the setting does not exist in this object, the object's class
		# and its ancestors will be checked.
		def setting(name)
			return @settings[name] if @settings.include?(name)
			return self.class.setting(name)
		end

		# Get whether a specific setting exists in this object.
		# If the setting does not exist in this object, the object's class
		# and its ancestors will be checked.
		def setting?(name)
			return true if @settings.include?(name)
			return self.class.setting?(name)
		end

		# Assign settings to this object from a hash.
		#   set username: 'Treeki', password: ARGV.last # secret!
		def set(hash)
			@settings.update(hash)
		end

		setup_settings


		on_connect do
			write_line "connect #{setting :username} #{setting :password}"
			write_line "color #{setting :color_code}" if setting?(:color_code)
			write_line "desc #{setting :description}" if setting?(:description)
			write_line "costume #{setting :costume}" if setting?(:costume)
		end

		on_enter_dream do
			# TODO: more handling for this
			write_line 'vascodagama' unless setting(:do_not_send_vascodagama)
		end
	end
end
