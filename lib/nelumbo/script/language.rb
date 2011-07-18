require 'set'

module Nelumbo
	module Script
		module LanguageDSL
			class LineProxy
				attr_reader :spec

				def initialize
					@spec = {}
				end

				def add(type, args)
					args.each{|arg| @spec[arg] = type}
				end

				def number_literal(*args);	add(:nliteral, args);	end
				def number_value(*args);	add(:nvalue, args);		end
				def number_variable(*args);	add(:nvariable, args);	end
				def string_literal(*args);	add(:sliteral, args);	end
				def string_value(*args);	add(:svalue, args);		end
				def string_variable(*args);	add(:svariable, args);	end
				def position_literal(*args); add(:pliteral, args);	end
				def position_value(*args);	add(:pvalue, args);		end
				def position_variable(*args);add(:pvariable, args);	end
			end

			class CategoryProxy
				attr_reader :lines

				def initialize
					@lines = {}
				end

				def add(number, spec)
					raise "line #{number} is already listed" if @lines[number]
					@lines[number] = spec
				end

				def line(*args, &block)
					if block_given?
						proxy = LineProxy.new
						proxy.instance_exec &block

						spec = proxy.spec
					else
						spec = {}
					end

					args.each do |number|
						if Range === number
							number.each{|n| add(n, spec)}
						else
							add(number, spec)
						end
					end
				end
			end
		end

		class Language
			class << self
				def spec_for_line(category, type)
					@line_spec[category][type]
				end

				def define_category(category, &block)
					proxy = LanguageDSL::CategoryProxy.new
					proxy.instance_exec &block

					@categories ||= Set.new
					@categories << category

					@line_spec ||= {}
					@line_spec[category] ||= {}
					@line_spec[category].update(proxy.lines)
				end

				def categories
					@categories ||= Set.new
				end
			end
		end
	end
end
