module Nelumbo
	# Nelumbo::MapAndScriptTracking is a module which can be included into a
	# bot to add a full map and DragonSpeak engine.
	#
	# The source .map and .ds files of the dream are required.
	# TODO: write docs and stuff for this..
	module MapAndScriptTracking
		module ClassMethods
			def setup_map_and_script_tracking
				set :do_not_send_vascodagama => true

				on_init_bot do
					init_source_dream_list
				end

				on_enter_dream do
					reset_mast_engine
					write_line 'dreambookmark 0'

					@mast_ds_count = -1
					@mast_waiting_for_url = true
				end

				on_message line: /^<img src='fsh:\/\/system\.fsh:86' \/> Lines of DragonSpeak/ do
					@mast_ds_count = data[:line][/Speak: (\d+)/,1].to_i
				end

				on_raw line: /^\]C0/ do
					halt unless @mast_waiting_for_url

					@mast_waiting_for_url = false
					init_mast_engine(data[:line].from(3), @mast_ds_count)
					write_line 'vascodagama'
				end
			end
		end


		def init_source_dream_list
			@mast_source_dreams = {}
		end

		def register_source_dream(url, path)
			@mast_source_dreams[url] = path
		end

		def reset_mast_engine
			puts "Resetting MaST engine. Placeholder!"
		end

		def init_mast_engine(url, line_count)
			puts "Initialising MaST engine. Placeholder! #{url} #{line_count}"
		end


		def self.included(klass)
			klass.extend ClassMethods
			klass.setup_map_and_script_tracking
		end
	end
end
