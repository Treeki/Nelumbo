module Nelumbo
	module Script
		class DragonSpeak < Language
			CAUSE = 0
			CONDITION = 1
			AREA = 3
			FILTER = 4
			EFFECT = 5

			EFFECTS = Set.new([AREA, FILTER, EFFECT])

			# CAUSES
			define_category(0) do
				# Dream Initialisation
				line(0)
				# Movement
				line(1)
				line(2) { number_literal :floor }
				line(3) { number_literal :object }
				line(7) { position_literal :position }
				# Turning
				line(4..6)
				# Arrival
				line(9..10)
				# Poses
				line(11..13)
				# Getting/Dropping/Using Objects
				line(15..16)
				line(17..19) { number_literal :object }
				# Dicerolls
				line(20..22) { number_literal :target_diceroll, :dice_count, :side_count }
				line(23..25) { number_literal :target_diceroll }
				# Speech/Emotes/Emits
				line(30)
				line(31..32) { string_literal :text }
				line(33)
				line(34..35) { string_literal :text }
				line(36)
				line(37..38) { string_literal :text }
				line(39)
				line(40..41) { string_literal :text }
				# Timers
				line(50) { number_literal :timer }
				# Cookies/Cookie Banks
				line(51..52) { number_literal :request }
				line(53..55)
				# AFK
				line(56..57)
				# Movement in Directions
				line(60..63)
				# Movement into Walls
				line(64) { number_literal :wall_shape }
				line(65) { number_literal :wall_texture }
				line(66) { number_literal :wall_shape, :wall_texture }
				# Player Idling
				line(70..73) { number_literal :seconds }
				# Ejection
				line(74)
				# Drop Failures
				line(78)
				line(79) { number_literal :object }
				# DS Buttons
				line(80) { number_literal :button }
				line(81)
				# iOS
				line(90)
				# Regular Timers
				line(100) { number_literal :interval, :offset }
				line(101) { number_literal :hours, :minutes }
				# Digo Magic
				line(200, 204)
				line(201, 205) { number_literal :floor }
				line(202, 206) { number_literal :object }
				line(203, 207) { position_literal :position }
				line(208) { number_literal :wall_shape }
				line(209) { number_literal :wall_texture }
				line(210) { number_literal :wall_shape, :wall_texture }
				line(220)
				# Digo Activation
				line(250..253, 270..274)
				# Digo Deactivation
				line(350..353, 370..374)
			end


			# CONDITIONS
			define_category(1) do
				# Movement
				line(2, 102) { number_value :floor }
				line(3, 103) { number_value :object }
				line(4, 5, 104, 105)
				line(7, 8, 107) { position_value :position }
				# Owner/Shared
				line(10, 11, 110, 111)
				# Player Facing
				line(12, 112) { position_value :position }
				line(13..16, 113..116)
				# Moved From
				line(17, 117) { number_value :floor }
				line(18, 118) { number_value :object }
				line(19, 119) { position_value :position }
				# Matching Floors/Objects
				line(30, 31, 130, 131) { position_value :first, :second }
				# Species/Digos/Gender
				line(20..29, 32..33, 39, 120..129, 132..133, 139)
				line(34, 134) { number_value :wing_type }
				line(35, 135)
				line(36..38, 136..138)
				# Floor/Object Checking based on Distance
				line(40..43, 140..143) { number_value :distance, :floor }
				line(44..47, 144..147) { number_value :first_distance, :second_distance, :floor }
				line(50..53, 150..153) { number_value :distance, :object }
				line(54..57, 154..157) { number_value :first_distance, :second_distance, :object }
				# Moving Through Walls
				line(60, 160) { number_value :wall_shape }
				line(61, 161) { number_value :wall_texture }
				line(62, 162) { number_value :wall_shape, :wall_texture }
				# Player
				line(70, 170) { string_value :name }
				line(71, 171)
				line(72, 172) { number_value :access_level }
				# String DS
				line(73, 173) { string_variable :variable }
				line(74, 174) { string_variable :one, :two }
				line(75, 175) { string_variable :variable; string_value :needle }
				line(76, 77, 176, 177) { string_variable :variable; number_value :value }
				# Players
				line(78, 82, 178, 182) { string_value :name }
				line(79, 179)
				line(80, 81, 180, 181) { position_value :top_left, :bottom_right }
				line(90, 92, 93, 190, 192, 193) { number_value :entry_code }
				line(91, 191) { number_value :entry_method }
				line(95, 195) { number_value :button }
				# Timer
				line(94, 194) { number_value :timer }
				# Variables
				line(200..202, 206) { number_variable :variable; number_value :value }
				line(203..205, 207) { number_variable :variable, :value }
				line(208, 209) { position_variable :first, :second }
				# Arrays
				line(250..261) do
					number_variable :array_base
					number_value :start_index, :count, :threshold, :value
				end
				line(310..313) { number_value :index; number_variable :array_base; number_value :value }
				# Species
				line(340, 440) { number_value :species }
				# PhoenixSpeak
				line(600..603, 620..623) { string_value :info; number_value :value }
				line(610..613) { string_value :info, :name; number_value :value }
				# Cookies
				line(700..703) { number_value :value }
				# Dice
				line(1000) { number_value :percentage }
				# Objects
				line(1002, 1004) { number_value :object }
				# Poses
				line(1005..1010)
				# Position Checking
				line(1011, 1012) { position_value :position; number_value :floor }
				line(1013, 1014) { position_value :position; number_value :object }
				line(1015, 1016) { position_value :position; number_value :wall_shape }
				line(1017, 1018) { position_value :position; number_value :wall_texture }
				line(1019, 1020) { position_value :position; number_value :wall_shape, :wall_texture }
				line(1100, 1101) { position_value :position }
				# Localspecies
				line(1200, 1202) { number_value :species }
			end


			# AREAS
			define_category(3) do
				# Everywhere
				line(1)
				# Specific areas
				line(2) { position_value :position }
				line(3, 4) { position_value :top_left, :bottom_right }
				# Relative to TF
				line(5, 6, 7, 10, 12)
				line(11, 13) { number_value :distance }
				# Can be Seen
				line(8)
				line(9) { position_value :position }
				# Lines
				line(14..17) { position_value :position; number_value :distance }
				# Relative to TF (again)
				line(20, 22)
				line(21, 23, 50..57) { number_value :distance }
				line(60..67) { number_value :first_distance, :second_distance }
				# Random
				line(500) { position_value :top_left, :bottom_right }
				line(501, 502)
				line(510, 520) { position_value :top_left, :bottom_right; number_value :floor }
				line(530, 540) { position_value :top_left, :bottom_right; number_value :object }
				line(511, 512, 521, 522) { number_value :floor }
				line(531, 532, 541, 542) { number_value :object }
			end


			# FILTERS
			define_category(4) do
				# Clear
				line(0)
				# Floors
				line(1, 2) { number_value :floor }
				# Objects
				line(3, 4) { number_value :object }
				line(7, 8)
				# Players
				line(5, 6)
				# Walkability
				line(9, 10)
				# Visible
				line(12, 13)
				line(14, 15) { position_value :position }
			end


			# EFFECTS
			define_category(5) do
				# Floor/Object Changes
				line(1) { number_value :floor }
				line(4) { number_value :object }
				line(2, 5) { number_value :from, :to }
				line(3, 6) { number_value :first, :second }
				# Sounds
				line(8, 9, 11, 12) { number_value :sound }
				line(10) { number_value :sound; position_value :position }
				# Player Movement
				line(14..17) { position_value :position }
				line(18)
				line(19) { number_value :distance }
				# Object Movement
				line(20) { number_value :distance }
				line(21..23) { position_value :position }
				# Floor Movement
				line(24, 25) { position_value :position }
				# Object/Floor Arithmetic
				line(26..29) { number_value :delta }
				# Midis
				line(30..32, 34) { number_value :midi }
				line(33) { number_value :midi; position_value :position }
				# Direct Floor/Object Changes
				line(40) { number_value :floor; position_value :position }
				line(41) { number_value :object; position_value :position }
				# Wall Changes
				line(42) { number_value :wall_shape }
				line(43) { number_value :wall_texture }
				line(44) { number_value :wall_shape, :wall_texture }
				line(45, 46) { number_value :first, :second }
				line(47) { number_value :first_shape, :first_texture, :second_shape, :second_texture }
				# Timer
				line(50) { number_value :timer, :delay }
				# Player Controls
				line(51) { position_value :position }
				line(52..55)
				line(56) { string_value :name }
				# More Wall Changes
				line(60, 61) { number_value :from, :to }
				line(62) { number_value :from_shape, :from_texture, :to_shape, :to_texture }
				line(63) { number_value :wall_shape; position_value :position }
				line(64) { number_value :wall_texture; position_value :position }
				line(65) { number_value :wall_shape, :wall_texture; position_value :position }
				line(66, 67, 68) { position_value :position }
				# Player Poses
				line(70..75)
				line(76, 77) { number_value :object }
				# Ejection
				line(78)
				# Player Relative Movement
				line(80..83) { number_value :distance }
				line(84..87) { number_value :first_distance, :second_distance }
				# Player Turning
				line(88..99)
				# Classic Mode
				line(100, 101)
				# Dream Control
				line(102, 103)
				line(104) { number_value :text_filter }
				line(105) { number_value :midi }
				line(106..111)
				line(112, 113)
				# DS Buttons
				line(180, 181, 190, 191) { number_value :button }
				line(182, 192) { number_value :button; position_value :position }
				line(183, 193) { number_value :button, :tab }
				line(184) { number_variable :variable }
				# Emits
				line(200, 201, 203, 204) { string_value :message }
				line(202) { string_value :message; position_value :position }
				line(205) { string_value :message, :name }
				# Other Text Stuff
				line(212) { string_value :prefix }
				line(213) { number_value :floor }
				line(214) { number_value :object }
				line(215) { string_value :message }
				# String Variables
				line(250) { string_variable :target; string_value :message }
				line(251) { string_variable :source, :target }
				line(252..256) { string_variable :target }
				line(257) { string_variable :target; string_value :message }
				line(258) { string_variable :target, :source }
				# String Chopping
				line(270, 271) { string_variable :target; number_value :remaining }
				line(272) { string_variable :target, :source; number_value :from, :to }
				line(273, 274) { string_variable :target; number_value :to_remove }
				# String Arrays
				line(275) { string_variable :array_base; number_value :index; string_variable :target }
				line(276) { string_variable :array_base; number_value :index; string_value :message }
				# Word Manipulation
				line(277) { string_variable :source, :target }
				line(278) { number_value :count; string_value :to_remove; string_variable :source }
				# Variables
				line(300) { number_variable :target; number_value :value }
				line(301) { number_variable :source, :target }
				# Variable Maths
				line(302, 304) { number_variable :target; number_value :delta }
				line(303, 305) { number_variable :target, :source }
				line(306) { number_variable :target; number_value :value }
				line(307) { number_variable :target, :source }
				line(308) { number_variable :target; number_value :value; number_variable :remainder }
				line(309) { number_variable :target, :source, :remainder }
				# Variable Arrays
				line(310) { number_variable :array_base; number_value :index; number_variable :target }
				line(311) { number_variable :array_base; number_value :index, :message }
				# Variable Dice Rolls
				line(312, 313) { number_variable :target; number_value :dice_count, :side_count, :delta }
				# Variable Data / Entry Codes
				line(314, 315, 317, 318) { number_variable :target }
				line(350, 351) { position_variable :target }
				line(316) { number_value :entry_code }
				# Variable Relative Movement
				line(352..355) { position_variable :target; number_value :distance }
				# Map Data/Positions
				line(380..383) { number_variable :target; position_value :position }
				line(384) { position_variable :target; position_value :position }
				# Arrays and Misc
				line(390) { number_value :start_index, :count; number_variable :array_base; number_value :value }
				line(399)
				# Cycling
				line(400, 410) { number_value :one, :two, :three }
				line(401, 411) { number_value :one, :two, :three, :four }
				line(402, 412) { number_value :one, :two, :three, :four, :five }
				# Map Shaking
				line(420, 421) { number_value :style, :time, :speed, :intensity }
				line(422, 423)
				# Animations
				line(430) { number_value :object, :step }
				line(431) { number_value :floor, :step }
				line(432) { number_value :wall_shape, :wall_texture, :step }
				line(433, 442) { number_value :button, :step }
				line(434) { number_value :object }
				line(435) { number_value :floor }
				line(436) { number_value :wall_shape, :wall_texture }
				line(437, 443) { number_value :button }
				line(438) { number_value :object, :delay }
				line(439) { number_value :floor, :delay }
				line(440) { number_value :wall_shape, :wall_texture, :delay }
				line(441, 444) { number_value :button, :delay }
				# Random Spots
				line(500) { position_variable :target; position_value :top_left, :bottom_right }
				line(501, 502) { position_variable :target }
				line(510, 520) { position_variable :target; number_value :floor; position_value :top_left, :bottom_right }
				line(511, 512, 521, 522) { position_variable :target; number_value :floor }
				line(530, 540) { position_variable :target; number_value :object; position_value :top_left, :bottom_right }
				line(531, 532, 541, 542) { position_variable :target; number_value :object }
				# PhoenixSpeak Memorise
				line(600, 602) { string_value :info; number_value :value }
				line(601) { string_value :info, :name; number_value :value }
				line(603, 605) { string_value :info, :value }
				line(604) { string_value :info, :name, :value }
				# PhoenixSpeak Remember
				line(610, 612) { string_value :info; number_variable :value }
				line(611) { string_value :info, :name; number_variable :value }
				line(613, 615) { string_value :info; string_variable :value }
				line(614) { string_value :info, :name; string_variable :value }
				# PhoenixSpeak Forget
				line(630, 633)
				line(631) { string_value :name }
				line(632, 634) { string_value :info }
				line(635) { string_value :info, :name }
				# PhoenixSpeak Databases
				line(650, 651)
				# Cookies
				line(700, 701) { number_value :cookie_count, :request; string_value :message }
				line(702, 703, 710) { number_value :cookie_count }
				line(704, 705) { number_variable :target }
				line(706, 707)
				line(708, 709) { string_value :message }
				# Move and Animate
				line(714..717) { position_value :position }
				line(718)
				line(719) { number_value :distance }
				line(780..783) { number_value :distance }
				line(784..787) { number_value :first_distance, :second_distance }
				# PS Timed Forget
				line(880) { number_value :days, :hours, :minutes, :seconds }
				# Misc
				line(1000)
				line(2000)
				# LocalSpecies
				line(1200, 1201) { number_value :species }
				line(1202, 1203)
			end
		end
	end
end
