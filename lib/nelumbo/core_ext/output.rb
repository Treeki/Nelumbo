class Integer
	def pluralize(word)
		if self == 1
			"#{self} #{word}"
		else
			"#{self} #{word.pluralize}"
		end
	end
end


