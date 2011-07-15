class String
	# Decodes a base 95 string into an integer.
	def decode_b95
		value = 0
		each_byte do |byte|
			value = (value * 95) + (byte - 32)
		end
		value
	end

	# Decodes a base 220 string into an integer.
	def decode_b220
		value = 0
		mult = 1
		each_byte do |byte|
			value += ((byte - 35) * mult)
			mult *= 220
		end
		value
	end
end


class Numeric
	# Encodes a number into a base 95 string of the specified length.
	def encode_b95(length)
		value = self
		output = ' '*length
		length.times do |index|
			output.setbyte(length - index - 1, (value % 95) + 32)
			value = (value / 95).floor
		end
		output
	end

	# Encodes a number into a base 220 string of the specified length.
	def encode_b220(length)
		value = self
		output = (' '*length).force_encoding(Encoding::BINARY)
		length.times do |index|
			output.setbyte(index, (value % 220) + 35)
			value = (value / 220).floor
		end
		output
	end
end

