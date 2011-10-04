require 'pry'

module Nelumbo::Graphics
	class FOX < BinData::Record
		class ExtData < BinData::Record
			endian :little
			uint16 :ext_data_size, value: lambda { ext_data.size + 2 }
			string :ext_data, length: :ext_data_size
		end

		endian :little
		string :magic, read_length: 4, initial_value: 'FSHX'
		int32 :version, initial_value: 1
		int32 :num_shapes, value: lambda { shapes.length }
		int32 :generator, initial_value: 31283128
		int32 :encryption, initial_value: 0
		skip length: 8

		array :shapes, initial_length: :num_shapes do
			uint16 :flags
			int16 :shape_no
			uint16 :num_frames, value: lambda { frames.length }
			uint16 :num_steps, value: lambda { steps.length }

			ext_data :extra, onlyif: lambda { version >= 3 }

			array :frames, initial_length: :num_frames do
				uint16 :format
				uint16 :width
				uint16 :height
				int16 :x
				int16 :y
				int16 :furre_x
				int16 :furre_y
				uint32 :image_data_size, value: lambda { image_data.size }

				ext_data :extra, onlyif: lambda { version >= 3 }

				hide :image_data
				binding.pry
				string :image_data, read_length: :image_data_size
			end

			array :steps, initial_length: :num_steps do
				uint16 :type
				uint16 :arg
				uint16 :counter_max
			end
		end
	end
end
