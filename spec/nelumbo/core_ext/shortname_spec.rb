require 'nelumbo'

describe "Nelumbo's String#to_shortname" do
	it "should return a sane shortname" do
		'#SO#SI CaT ~ 12345 #SI#SO'.to_shortname.should == 'sosicat12345siso'
	end

	it "should handle less-than/greater-than signs" do
		'<a>'.to_shortname.should == 'a'
		'&lt;a&gt;'.to_shortname.should == 'a'
	end

	it "should handle accented characters" do
		'&eth;&agrave;&ntilde;&eacute;&yuml;'.to_shortname.should == 'daney'
	end
end

