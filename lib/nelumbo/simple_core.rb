module Nelumbo
	# Nelumbo::SimpleCore implements a core which only handles one bot.
	#
	# The Nelumbo::Bot class uses SimpleCore as a default if no core is
	# specified. When the bot disconnects, the core will automatically stop.
	class SimpleCore < SelectCore
		private :add_bot, :remove_bot

		def run(bot)
			add_bot bot
			super()
		end

		def remove_bot(bot)
			# if the only bot is removed, then we can stop running entirely
			@running = false
			super
		end
	end
end

