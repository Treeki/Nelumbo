module Nelumbo
	module Script
		# This class turns a list of script lines (as produced by
		# Nelumbo::Script::LineParser) into a set of blocks and performs other
		# semantic analysis, including creating a variable table.
		#
		# Each block consists of one or more triggers and a set of effects.
		# Each trigger consists of a cause and zero or more conditions.
		#
		class TreeParser
			def initialize(language)
				@language = language
			end

			def parse(line_array)
				reset!

				line_array.each do |line|
					
					case line[:category]
					when @language::CAUSE
						if new_block?
							make_new_block(line)
						else
							make_new_trigger(line)
						end
					when @language::CONDITION
						add_condition(line)
					when *@language::EFFECTS
						add_effect(line)
					end
				end
			end

			def line_count
				@number
			end

			attr_reader :blocks
			
			private
			def reset!
				@blocks = []
				@current_block = nil
				@current_trigger = nil
				@number = 0
				@variables_by_name = {}
				@svariables_by_name = {}
				@next_variable = 0
				@next_svariable = 0
			end

			def assign_line_number
				@number += 1
			end

			def add_variable(v)
				return if variable?(v)

				@variables_by_name[v[:name]] = @next_variable
				count = v[:array_count] || 1
				@next_variable += count * 2
			end

			def add_string_variable(v)
				return if string_variable?(v)

				@svariables_by_name[v[:name]] = @next_svariable
				count = v[:array_count] || 1
				@next_svariable += count
			end

			def variable?(v)
				if Hash === v
					@variables_by_name.key?(v[:name])
				else
					@variables_by_name.key?(v)
				end
			end

			def string_variable?(v)
				if Hash === v
					@svariables_by_name.key?(v[:name])
				else
					@svariables_by_name.key?(v)
				end
			end

			def index_of_variable(v)
				if Hash === v
					@variables_by_name[v[:name]] + ((v[:part] == :y) ? 1 : 0)
				else
					@variables_by_name[v]
				end
			end

			def index_of_string_variable(v)
				if Hash === v
					@svariables_by_name[v[:name]]
				else
					@svariables_by_name[v]
				end
			end

			def process_line(line)
				line[:number] = assign_line_number

				line.values.each do |v|
					case v[:type]
					when :variable
						add_variable(v)
					when :string_variable
						add_string_variable(v)
					end
				end
			end

			def new_block?
				@current_block.nil? or !@current_block[:effects].empty?
			end

			def make_new_trigger(cause)
				cause[:number] = assign_line_number
				@current_trigger = {cause: cause, conditions: []}
				@current_block[:triggers] << @current_trigger
			end

			def make_new_block(cause)
				@current_block = {triggers: [], effects: []}
				make_new_trigger(cause)
				@blocks << @current_block
			end

			def add_condition(condition)
				condition[:number] = assign_line_number
				@current_trigger[:conditions] << condition
			end

			def add_effect(effect)
				effect[:number] = assign_line_number
				@current_block[:effects] << effect
			end
		end
	end
end
