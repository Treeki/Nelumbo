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
					process_line line

					if line[:type] == :comment
						if /\[NB\] (?<command>.*)$/ =~ line[:string]
							@next_annotation = command
						end
						next
					end

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

					if @next_annotation
						line[:annotation] = @next_annotation
						@next_annotation = nil
					end
				end
			end

			def line_count
				@number
			end

			attr_reader :blocks
			attr_reader :variables_by_name
			attr_reader :svariables_by_name
			
			public
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
					@variables_by_name[v[:name]] + ((v[:part] == ?y) ? 1 : 0)
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
				@next_annotation = nil
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

			def process_line(line)
				line.values.each do |v|
					process_value(v)
				end

				line[:number] = assign_line_number
			end

			def process_value(value)
				if Array === value
					value.each { |v| process_value(v) }

				elsif Hash === value
					case value[:type]
					when :variable
						add_variable(value)
					when :string_variable
						add_string_variable(value)
					end
				end
			end

			def new_block?
				@current_block.nil? or !@current_block[:effects].empty?
			end

			def make_new_trigger(cause)
				@current_trigger = {cause: cause, conditions: []}
				@current_block[:triggers] << @current_trigger
			end

			def make_new_block(cause)
				@current_block = {triggers: [], effects: []}
				make_new_trigger(cause)
				@blocks << @current_block
			end

			def add_condition(condition)
				@current_trigger[:conditions] << condition
			end

			def add_effect(effect)
				@current_block[:effects] << effect
			end
		end
	end
end
