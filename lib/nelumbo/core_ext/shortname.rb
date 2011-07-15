ShortnameAccents = {
	'a' => ['&agrave;', '&aacute;', '&acirc;', '&atilde;', '&auml;', '&aring;', '&aelig;'],
	'c' => ['&ccedil;'],
	'd' => ['&eth;'],
	'e' => ['&egrave;', '&eacute;', '&ecirc;', '&euml;'],
	'i' => ['&igrave;', '&iacute;', '&icirc;', '&iuml;'],
	'n' => ['&ntilde;'],
	'o' => ['&ograve;', '&oacute;', '&ocirc;', '&otilde;', '&ouml;', '&oric;', '&oslash;'],
	'u' => ['&ugrave;', '&uacute;', '&ucirc;', '&uuml;'],
	'y' => ['&yacute;', '&yuml;']
}

class String
	# Convert a name to a shortname, taking into account HTML entities.
	def to_shortname
		name = self.downcase
		name.gsub! '&lt;', '<'
		name.gsub! '&gt;', '>'

		if name.include? '&'
			ShortnameAccents.each_pair do |real_char, chars|
				chars.each {|entity| name.gsub! entity, real_char}
			end
		end

		name.delete '^0-9a-z'
	end
end

