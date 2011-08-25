module Nelumbo
	module Script
		# This class transforms a set of tokens (as obtained from
		# Nelumbo::Script::Tokenizer) into a list of DS lines and comments.
		#
		# Since DragonSpeak's syntax is dependent on which lines are used and
		# the parameters they have, this class requires a subclass of
		# Nelumbo::Script::Language.
		#
		class LineParser
			def initialize(language, token_array)
				@language = language
				@tokens = token_array
				@token_enum = token_array.each
				@debug_state = nil
			end

			def lines
				enum = Enumerator.new do |y|
					@line_yielder = y

					catch :ds_complete do
						loop do
							token = fetch_token

							if token[:type] == :number and @language.categories.include?(token[:number])
								try_line token
							elsif token[:type] == :version
								@line_yielder << token
							else
								@line_yielder << {type: :error, cause: :unexpected_token, token: token}
							end
						end
					end
				end

				enum

			rescue StopIteration
				return enum
			end

			def each_line
				return lines unless block_given?

				lines.each{|line| yield line}
			end


			def try_line(initial_token)
				@debug_state = nil
				category = initial_token[:number]
				type = fetch_token[:number]
				@debug_state = "#{category}:#{type}"

				spec = @language.spec_for_line(category, type)
				line = {category: category, type: type}

				if spec.nil?
					@line_yielder << {type: :error, cause: :unknown_line}.merge(line)
					return
				end

				spec.each_pair do |name, type|
					value = send("fetch_#{type}".to_sym)
					line[name] = value
				end

				@line_yielder << line
			end

			# Returns the next non-comment token. Comment tokens are yielded.
			def fetch_token
				loop do
					token = @token_enum.next
					if token[:type] == :comment
						throw(:ds_complete) if /^Endtriggers/ =~ token[:string]

						@line_yielder << token
					else
						return token
					end
				end

				throw :ds_complete
			end

			# Returns the next non-comment token if it falls into a specified
			# group. If not, then it returns nil and yields an error.
			def fetch_token_restricted(groups, name)
				token = fetch_token
				return token if groups.include?(token[:type])

				@line_yielder << {type: :error, cause: :unexpected_token, token: token, wanted: name}

				nil
			end


			def fetch_nliteral
				fetch_token_restricted([:number], :nliteral)
			end

			def fetch_nvalue
				fetch_token_restricted([:number, :variable, :variable_pointer], :nvalue)
			end

			def fetch_nvariable
				fetch_token_restricted([:variable, :variable_pointer], :nvariable)
			end

			def fetch_sliteral
				fetch_token_restricted([:string], :sliteral)
			end

			def fetch_svalue
				fetch_token_restricted([:number, :string, :variable, :string_variable, :variable_pointer], :svalue)
			end

			def fetch_svariable
				fetch_token_restricted([:number, :string_variable, :variable_pointer], :svariable)
			end

			def fetch_pliteral
				x = fetch_token_restricted([:number], :pliteral_x)
				y = fetch_token_restricted([:number], :pliteral_y)
				[x,y]
			end

			def fetch_pvalue
				x = fetch_token_restricted([:number, :variable, :variable_pointer], :pvalue_x)

				case x[:type]
				when :variable_pointer
					# note: variable pointers ALWAYS expand
					y = x.merge(number: x[:number]+1)

				when :variable
					if x[:can_expand]
						y = x.merge(part: 'y')
					else
						y = fetch_token_restricted([:number, :variable, :variable_pointer], :pvalue_y)
					end

				when :number
					y = fetch_token_restricted([:number, :variable, :variable_pointer], :pvalue_y)

				end

				[x,y]
			end

			def fetch_pvariable
				# position variable references are the same as regular variable
				# references for our purposes
				fetch_nvariable
			end
		end
	end
end
