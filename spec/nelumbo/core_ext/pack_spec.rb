require 'nelumbo'

describe "Nelumbo's String#furc_unpack" do
	it "should unpack base 95 integers" do
		'%'.furc_unpack('a').should == 5
		'$%'.furc_unpack('b').should == 385
		'!$%'.furc_unpack('c').should == 9410
		'*!$%'.furc_unpack('d').should == 8583160
	end

	it "should unpack base 220 integers" do
		'%'.furc_unpack('A').should == 2
		'$%'.furc_unpack('B').should == 441
		'#$%'.furc_unpack('C').should == 97020
		'*#$%'.furc_unpack('D').should == 21344407
	end

	it "should unpack base 95 strings" do
		'%abcdefg'.furc_unpack('s').should == 'abcde'
		'%abcde$fghijkl'.furc_unpack('ss').should == ['abcde','fghi']
	end

	it "should unpack base 220 strings" do
		'(abcdefg'.furc_unpack('S').should == 'abcde'
		'(abcde)fghijklmn'.furc_unpack('SS').should == ['abcde','fghijk']
	end

	it "should unpack colour codes" do
		'tABCDEFGHIJ'.furc_unpack('k').should == 'tABCDEFGHIJ'
		'tABCDEFGHIJKLM'.furc_unpack('K').should == 'tABCDEFGHIJKLM'
	end

	it "should handle the skip character correctly" do
		'abc%abc$%abc'.furc_unpack('xxxaxxxbxxx').should == [5,385]
	end

	it "should not screw up when all of these are used together" do
		result = '$%$%%abcde(abcdetABCDEFGHIJKLMqq%'.furc_unpack('bBsS!xxa')
		result.should == [385,441,'abcde','abcde','tABCDEFGHIJKLM',5]
	end

	it "should handle encodings properly" do
		"\xB0\xBD".furc_unpack('B').should == 34021
		"\xB0\xBD(abcdefg".furc_unpack('BS').should == [34021,'abcde']

		test_string = 'a' * 154
		("\xB0\xBD\xBD"+test_string).furc_unpack('BS').should == [34021,test_string]
	end
end

describe "Nelumbo's Array#furc_pack" do
	it "should pack base 95 integers" do
		[5].furc_pack('a').should == '%'
		[385].furc_pack('b').should == '$%'
		[9410].furc_pack('c').should == '!$%'
		[8583160].furc_pack('d').should == '*!$%'
	end

	it "should pack base 220 integers" do
		[2].furc_pack('A').should == '%'
		[441].furc_pack('B').should == '$%'
		[97020].furc_pack('C').should == '#$%'
		[21344407].furc_pack('D').should == '*#$%'
	end

	it "should pack base 95 strings" do
		['abcde'].furc_pack('s').should == '%abcde'
		['abcde','fghi'].furc_pack('ss').should == '%abcde$fghi'
	end

	it "should pack base 220 strings" do
		['abcde'].furc_pack('S').should == '(abcde'
		['abcde','fghijk'].furc_pack('SS').should == '(abcde)fghijk'
	end

	context "when packing colour codes" do
		it "should not mangle them" do
			['tABCDEFGHIJ'].furc_pack('k').should == 'tABCDEFGHIJ'
			['tABCDEFGHIJKLM'].furc_pack('K').should == 'tABCDEFGHIJKLM'
		end

		it "should trim colour codes which are too long" do
			['tABCDEFGHIJKLMNOPQRSTUV'].furc_pack('k').should == 'tABCDEFGHIJ'
			['tABCDEFGHIJKLMNOPQRSTUV'].furc_pack('K').should == 'tABCDEFGHIJKLM'
		end

		it "should pad colour codes which are too short" do
			['tABCDEF'].furc_pack('k').should == 'tABCDEF####'
			['tABCDEF'].furc_pack('K').should == 'tABCDEF#######'
		end
	end

	it "should handle the direct character correctly" do
		['a', 'bb', 'ccc'].furc_pack('xxx').should == 'abbccc'
	end

	it "should not screw up when all of these are used together" do
		result = [385,441,'abcde','abcde','tABCDEFGHIJKLM','q','q',5]
		result.furc_pack('bBsS!xxa').should == '$%$%%abcde(abcdetABCDEFGHIJKLMqq%'
	end

	it "should handle encodings properly" do
		[34021].furc_pack('B').should == "\xB0\xBD"
		[34021,'abcde'].furc_pack('BS').should == "\xB0\xBD(abcde"

		test_string = 'a' * 154
		[34021,test_string].furc_pack('BS').should == "\xB0\xBD\xBD"+test_string
	end
end

