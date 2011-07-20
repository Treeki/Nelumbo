module Nelumbo
	module Script
		# This class converts a DragonSpeak script (well, the effect bits of
		# it) into a Ruby class. Liberal amounts of magic are involved. Beware.
		#
		# I doubt I'll finish this anyway.
		#
		class CodeGenerator
			def initialize(language)
				@language = language
				@methods = {}
				@method_num = 0
			end

			attr_reader :language

			# Creates code for the effects in the passed script block and
			# returns the method name as a symbol.
			def generate_for(block)
				method_name = "dsb#{@method_num += 1}".to_sym

				@method_code = ''

				# TODO: refactor this into a per-language bit somehow

				block[:effects].each do |line|
					@current_line = line
					case line[:category]
					when 3 then do_area(line)
					when 4 then do_filter(line)
					when 5 then do_effect(line)
					end
				end

				@methods[method_name] = @method_code
				method_name
			end


			# Used in as_ruby code to get the current line
			def line
				@current_line
			end

			# Used in as_ruby code to add some code
			def write(code)
				@method_code << code
				@method_code << "\n"
			end

			def value(arg, part=:none)
				value_direct(@current_line[arg], part)
			end

			def value_direct(value, part=:none)
				if Array === value
					# position, fun
					case part
					when :x
						value_direct(value.first)
					when :y
						value_direct(value.last)
					else
						[value_direct(value.first), value_direct(value.last)]
					end
				else
					case value[:type]
					when :number
						value[:number].to_s
					when :variable_pointer, :variable
						"@variable[#{ref_direct value, part}]"
					end
				end
			end

			def ref(arg, part=:none)
				ref_direct(@current_line[arg], part)
			end

			def ref_direct(value, part=:none)
				if Array === value
					raise "this should not happen"
				else
					case value[:type]
					when :number
						raise "this should not happen"
					when :variable_pointer
						value[:number] + ((part == :y) ? 1 : 0)
					when :variable
						part_str = (part == :y) ? '_y' : ''
						(value[:name] + part_str).inspect
					end
				end
			end

			def assign_var(target, value)
				write "@variable[#{target}] = #{value}"
			end

			private :line, :write, :ref


			# Returns the code for this class.
			# It is a Ruby expression to create an anonymous class that has
			# the required methods.
			def finalize
				buffer = <<END
Class.new do
	include Nelumbo::Script::Runtime
	def initialize(context)
		@context = context
	end

END
				@methods.each_pair do |name, code|
					buffer << "def #{name}\n#{code}\nend\n\n"
				end
				buffer << "end\n"
				buffer
			end




			private

			def do_area(line)
				case line[:type]
				when 1
					write "@area = nil"
				when 2
					write "@area = [#{value :position, :x}, #{value :position, :y}]"
				when 3
					write "@area = [:diamond, " +
						"#{value :top_left, :x}, #{value :top_left, :y}, " +
						"#{value :bottom_right, :x}, #{value :bottom_right, :y}]"
				when 4
					write "@area = [:rectangle, " +
						"#{value :top_left, :x}, #{value :top_left, :y}, " +
						"#{value :bottom_right, :x}, #{value :bottom_right, :y}]"
				when 5
					write "@area = @context.moved_from"
				when 6
					write "@area = @context.moved_to"
				when 7
					write "@area = @context.successfully_moved? ? @context.moved_to : @context.moved_from"
				when 8
					write "@area = [:visible, *(@context.successfully_moved? ? @context.moved_to : @context.moved_from)]"
				when 9
					write "@area = [:visible, #{value :position, :x}, #{value :position, :y}]"
				end
			end

			def do_filter(line)

			end

			def do_effect(line)

			end
		end
	end
end
