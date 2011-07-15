class String
	# Unpacks a Furcadia protocol string. TODO: document this.
	def furc_unpack(format)
		output = []

		offset = 0
		format.each_char do |piece|
			case piece
			when 'x'
				# Skip
				offset += 1

			when '!'
				# Colour code
				cc_format = self[offset]
				if cc_format == 't'
					output << slice(offset, 13)
					offset += 13
				else
					raise "unsupported colour code type: #{cc_format}"
				end

			when 's', 'S'
				# Base 95/220 string: lowercase = 95, uppercase = 220
				base = (piece == 'S') ? 35 : 32
				length = getbyte(offset) - base
				output << slice(offset+1, length)
				offset += length + 1

			when 'a'..'f'
				# Base 95 big-endian integer: a = 1 digit, b = 2 digits, c = 3 digits, ...
				digits = piece.ord - 96
				output << slice(offset, digits).decode_b95
				offset += digits

			when 'A'..'F'
				# Base 220 little-endian integer: A = 1 digit, B = 2 digits, C = 3 digits, ...
				digits = piece.ord - 64
				output << slice(offset, digits).decode_b220
				offset += digits

			end
		end

		return output.first if output.size == 1
		output
	end

end



