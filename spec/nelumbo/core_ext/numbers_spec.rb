require 'nelumbo'

describe "Nelumbo's base 95 encoding/decoding methods" do
	it "should encode numbers into strings correctly" do
		0.encode_b95(1).should == 32.chr
		1.encode_b95(1).should == 33.chr
		94.encode_b95(1).should == 126.chr

		95.encode_b95(2).should == 33.chr + 32.chr
		
		1.encode_b95(4).should == 32.chr + 32.chr + 32.chr + 33.chr
		4546432.encode_b95(4).should == '%<h1'
	end

	it "should overflow when encoding a number that is too large" do
		95.encode_b95(1).should == 32.chr
		9025.encode_b95(2).should == 32.chr * 2
	end


	it "should decode strings into numbers correctly" do
		32.chr.decode_b95.should == 0
		33.chr.decode_b95.should == 1
		126.chr.decode_b95.should == 94

		(33.chr + 32.chr).decode_b95.should == 95

		(32.chr + 32.chr + 32.chr + 33.chr).decode_b95.should == 1
		'%<h1'.decode_b95.should == 4546432
	end
end

describe "Nelumbo's base 220 encoding/decoding methods" do
	it "should encode numbers into strings correctly" do
		0.encode_b220(1).should == 35.chr
		1.encode_b220(1).should == 36.chr
		219.encode_b220(1).should == 254.chr(Encoding::BINARY)

		220.encode_b220(2).should == 35.chr + 36.chr

		1.encode_b220(4).should == 36.chr + 35.chr + 35.chr + 35.chr
		716624983.encode_b220(4).should == 'beef'
	end

	it "should overflow when encoding a number that is too large" do
		220.encode_b220(1).should == 35.chr
		48400.encode_b220(2).should == 35.chr * 2
	end


	it "should decode strings into numbers correctly" do
		35.chr.decode_b220.should == 0
		36.chr.decode_b220.should == 1
		254.chr.decode_b220.should == 219

		(35.chr + 36.chr).decode_b220.should == 220

		(36.chr + 35.chr + 35.chr + 35.chr).decode_b220.should == 1
		'beef'.decode_b220.should == 716624983
	end
end

