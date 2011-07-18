require 'set'

module Nelumbo
	module Script
		class Tokenizer
			NEW_LINE_CHARS = Set.new(["\r", "\n"])
			VARIABLE_END_CHARS = Set.new([' ', '(', ')', '[', ']', '.', ',', "\r", "\n"])
			STRING_VARIABLE_APPROVED_CHARS = Set.new(('A'..'Z').to_a + ('a'..'z').to_a + ('0'..'9').to_a)

			def initialize(input_stream)
				@input = input_stream

				@current_state = :awaiting_version
			end

			def reset_state
				@next_var_can_expand = false
				true
			end

			def each_token
				@input.each_char do |char|
					# DIAF, carriage returns
					next if char == "\r"

					case @current_state
					when :awaiting_version
						next if NEW_LINE_CHARS.include?(char)
						@current_state = :version
						@version = char

					when :version
						if NEW_LINE_CHARS.include?(char)
							yield({type: :version, string: @version})
							@current_state = :nothing
						else
							@version += char
						end

					when :nothing
						case char
						when '-', '0'..'9'
							@current_state = :number
							@number_string = char 
						when '*'
							@current_state = :comment
							@comment = ''
						when '('
							@next_var_can_expand = true
							next
						when '%'
							@current_state = :variable
							@variable = {type: :variable, name: ''}
							@variable[:can_expand] = true if @next_var_can_expand
						when '~'
							@current_state = :string_variable
							@variable = {type: :string_variable, name: ''}
						when '@'
							@current_state = :variable_pointer
							@pointer_string = ''
						when '{'
							@current_state = :string
							@string = ''
						end

					when :comment
						if char == "\n"
							@current_state = :nothing
							yield({type: :comment, string: @comment})
						else
							@comment += char
						end

					when :number
						if ('0'..'9') === char
							@number_string += char 
						else
							if @number_string != '-'
								yield({type: :number, number: @number_string.to_i})
							end

							@current_state = :nothing
							reset_state and redo
						end

					when :variable
						if VARIABLE_END_CHARS.include?(char)
							case char
							when '['
								@current_state = :variable_array_def
								@array_string = ''
							when '.'
								@current_state = :variable_part
							else
								@current_state = :variable_check_expansion
							end
						else
							@variable[:name] += char
						end

					when :variable_array_def
						case char
						when '0'..'9'
							@array_string += char
						when ']'
							@variable[:array_count] = @array_string.to_i
							@current_state = :variable_might_have_part_after_array
						else
							@variable[:array_count] = @array_string.to_i
							@current_state = :variable_check_expansion
						end

					when :variable_might_have_part_after_array
						if char == '.'
							@current_state = :variable_part
						else
							@current_state = :variable_check_expansion
							reset_state and redo
						end

					when :variable_part
						if char == 'x' or char == 'y'
							@variable[:part] = char
							@current_state = :variable_check_expansion
						end

					when :variable_check_expansion
						if @variable[:can_expand] and char != ')'
							# can't expand it if it doesn't end with )
							@variable.delete(:can_expand)
						end

						@current_state = :nothing
						yield @variable

						# if it wasn't ), then pass the character on in case
						# it might need to be parsed
						reset_state and redo unless char == ')'

					when :string
						if char == '}' or NEW_LINE_CHARS.include?(char)
							@current_state = :nothing
							yield({type: :string, string: @string})
						else
							@string += char
						end

					when :string_variable
						unless STRING_VARIABLE_APPROVED_CHARS.include?(char)
							if char == '['
								@current_state = :string_variable_array_def
								@array_string = ''
							else
								@current_state = :nothing
								yield @variable
							end
						end

					when :string_variable_array_def
						if ('0'..'9') === char
							@array_string += char
						else
							@variable[:array_count] = @array_string.to_i
							@current_state = :nothing
							yield :variable

							reset_state and redo if char != ']'
						end

					when :variable_pointer
						if ('0'..'9') === char
							@pointer_string += char
						else
							@current_state = :nothing
							yield({type: :variable_pointer, number: @pointer_string.to_i})

							reset_state and redo
						end
					end

					reset_state
				end

			rescue EOFError
				return
			end
		end
	end
end

