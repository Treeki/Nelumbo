module Nelumbo
	# The Nelumbo::SelectCore class can be used as a core for any number of
	# bots, multiplexing them using the IO#select function.
	#
	# The running bots can be changed on-the-fly.
	#
	class SelectCore
		TIMER_GRANULARITY = 0.1

		def initialize
			@bots = []
			@bot_lookup_by_socket = {}
			@bot_lookup_by_object = {}
			@running = false
			@write_interval = 0.1
		end

		attr_reader :running

		def add_bot(bot_class)
			bot = {object: bot_class}
			@bots << bot

			h_write_line = proc { |line| write_line(bot, line) }
			h_disconnect = proc { remove_bot(bot) }
			bot_class.set_core_hooks(h_write_line, h_disconnect)

			bot[:read_buffer] = ''
			bot[:lines_to_write] = []
			bot[:can_write_at] = Time.now
			# TODO: make this configurable
			bot[:socket] = TCPSocket.new('lightbringer.furcadia.com', 6500)

			@bot_lookup_by_socket[bot[:socket]] = bot
			@bot_lookup_by_object[bot[:object]] = bot

			bot_class.bot_started
		end

		def remove_bot(bot)
			bot[:object].bot_ended
			bot[:socket].close

			@bots.delete bot
			@bot_lookup_by_socket.delete bot[:socket]
			@bot_lookup_by_object.delete bot[:object]
		end

		def really_write_line(bot, line)
			line = line + "\n"
			offset = 0
			
			puts "[[ #{Time.now.to_f} SENT #{line.inspect} ]]"
			while offset < line.length
				offset += bot[:socket].write((offset == 0) ? line : line.from(offset))
			end
		end

		def write_line(bot, line)
			bot[:lines_to_write] << line
		end

		def run
			# a very weird optimisation, but it actually works! I hope.
			tgran = TIMER_GRANULARITY.to_r

			# this is where all the fun happens
			@running = true
			target_time = Time.now + tgran

			while @running
				current_time = Time.now
				while current_time >= target_time
					target_time += tgran

					@bots.each do |b|
						# TODO: store the last tick in the Bot object
						b[:object].timer_tick
					end
				end

				# do the actual processing
				read_sockets = @bots.map { |b| b[:socket] }
				arrays = IO.select(read_sockets, read_sockets, nil, (target_time - Time.now).to_f.abs)
				read, write = arrays

				next if arrays.nil?

				read.each do |socket|
					bot = @bot_lookup_by_socket[socket]

					# this one had some stuff
					data = bot[:read_buffer] + socket.recvfrom(512)[0]
					packet = data.split("\n")

					if data[-1] == "\n"
						# we have a complete line, nothing to buffer
						bot[:read_buffer] = ''
					else
						if data.length > 0
							# put the last (incomplete) line into the buffer
							bot[:read_buffer] = packet.pop
						else
							remove_bot bot
							next
						end
					end

					packet.each do |line|
						bot[:object].line_received(line) unless line.empty?
					end
				end

				write.each do |socket|
					bot = @bot_lookup_by_socket[socket]
					next if bot.nil?

					write_buffer = bot[:lines_to_write]
					if !write_buffer.empty? and bot[:can_write_at] <= Time.now
						really_write_line(bot, write_buffer.shift)
						bot[:can_write_at] = Time.now + @write_interval
					end
				end
			end
		end
	end
end

