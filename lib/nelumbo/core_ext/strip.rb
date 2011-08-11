class String
	def furc_strip
		self.gsub(/<(\/)?(font|img|channel|desc|name|b|i|u|a)(.*?)>/, '')
	end
end
