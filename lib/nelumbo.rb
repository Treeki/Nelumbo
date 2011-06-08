require 'nelumbo/base'

# TODO: figure out how well this works...
nelumbo_caller = caller.reject{|c| c =~ /^<internal/}.first
nelumbo_caller_file_name = nelumbo_caller.split(':').first
nelumbo_caller_file_name.gsub! /\.rb$/, ''

at_exit do
	bot_module = nelumbo_caller_file_name.camelize.constantize
	bot_module::Bot.new.run
end
