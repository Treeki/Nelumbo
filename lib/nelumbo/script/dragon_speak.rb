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
				# Drop Object
				line(8)
				# Arrival
				line(9..10)
				# Poses
				line(11..13)
				# Getting/Dropping/Using Objects
				line(14)
				line(15..16)
				line(17..19) { number_literal :object }
				# Dicerolls
				line(20..22) { number_literal :target_diceroll, :dice_count, :side_count }
				line(23..25) { number_literal :target_diceroll }
				# Regions
				line(26) { number_literal :region }
				line(27) { number_literal :low_region, :high_region }
				line(28) { number_literal :from_region, :to_region }
				# Effects
				line(29) { number_literal :effect }
				# Speech/Emotes/Emits
				line(30)
				line(31..32) { string_literal :text }
				line(33)
				line(34..35) { string_literal :text }
				line(36)
				line(37..38) { string_literal :text }
				line(39)
				line(40..41) { string_literal :text }
				# Movement
				line(42) { number_literal :lighting }
				line(43) { number_literal :ambience }
				# Timers
				line(49)
				line(50) { number_literal :timer }
				# Cookies/Cookie Banks
				line(51..52) { number_literal :request }
				line(53..55)
				# AFK
				line(56..57)
				# Summoning
				line(58)
				# Dream Portal Placement
				line(59)
				# Movement in Directions
				line(60..63)
				# Movement into Walls
				line(64) { number_literal :wall_shape }
				line(65) { number_literal :wall_texture }
				line(66) { number_literal :wall_shape, :wall_texture }
				# Movement from Floor/Object
				line(67) { number_literal :floor }
				line(68) { number_literal :object }
				# Movement into Region
				line(69) { number_literal :region }
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
				# Dialogs
				line(85..87) { number_literal :request }
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
				line(240)
				# Digo Activation
				line(250..253, 270..274)
				# Digo Deactivation
				line(350..353, 370..374)
				# Desctags
				line(400, 402..405) { number_literal :desctag }
				line(401)
				# Dream Portal
				line(451)
			end


			# CONDITIONS
			define_category(1) do
				# Movement
				line(2, 102) { number_value :floor }
				line(3, 103) { number_value :object }
				line(4, 5, 104, 105)
				line(7, 8, 107, 108) { position_value :position }
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
				# Facing Towards
				line(58, 158) { number_value :object }
				line(59, 159) { number_value :floor }
				# Moving Through Walls
				line(60, 160) { number_value :wall_shape }
				line(61, 161) { number_value :wall_texture }
				line(62, 162) { number_value :wall_shape, :wall_texture }
				# Lighting/ambience
				line(63, 64, 163, 164) { number_value :lighting }
				line(65, 165) { position_value :position; number_value :lighting }
				line(66, 67, 166, 167) { number_value :ambience }
				line(68, 168) { position_value :position; number_value :ambience }
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
				# Regions
				line(83, 183) { number_value :region }
				line(84, 184) { number_value :low_region, :high_region }
				line(85, 185) { number_value :region }
				line(86, 186) { position_value :position; number_value :region }
				line(87, 187) { number_value :region }
				line(88, 188) { number_value :region, :threshold }
				# Players, Again
				line(90, 92, 93, 190, 192, 193) { number_value :entry_code }
				line(91, 191) { number_value :entry_method }
				line(95, 195) { number_value :button }
				# Timer
				line(94, 194) { number_value :timer }
				# Effects
				line(96, 97, 196, 197) { number_value :effect }
				line(98, 198) { position_value :position; number_value :effect }
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
				line(341, 441) { number_value :digo_category }
				line(350, 450)
				line(351, 352, 353, 451) { number_value :size }
				line(361, 461)
				# PhoenixSpeak
				line(600..603, 620..623) { string_value :info; number_value :value }
				line(605, 606, 625, 626) { string_value :info }
				line(610..613) { string_value :info, :name; number_value :value }
				line(615, 616) { string_value :info, :name }
				line(630, 631) { string_value :info, :needle }
				line(632..635) { string_value :info; number_value :value }
				line(680, 681)
				# Cookies
				line(700..703) { number_value :value }
				# Dream Portals
				line(800..803) { number_value :portal_type }
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
				# Remaps
				line(1250, 1251) { number_value :remap, :id }
				# Desctags
				line(1300..1302) { number_value :count, :desctag }
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
				# Dream Portal
				line(18)
				# Relative to TF (again)
				line(20, 22)
				line(21, 23, 50..57) { number_value :distance }
				# Regions
				line(30, 31) { number_value :region }
				line(32, 33) { number_value :low_region, :high_region }
				# Relative to TF (yet again)
				line(60..67) { number_value :first_distance, :second_distance }
				# Dream Portals
				line(70, 71)
				# Random
				line(500) { position_value :top_left, :bottom_right }
				line(501, 502)
				line(510, 520) { position_value :top_left, :bottom_right; number_value :floor }
				line(530, 540) { position_value :top_left, :bottom_right; number_value :object }
				line(550, 560) { position_value :top_left, :bottom_right; number_value :region }
				line(570, 580) { position_value :top_left, :bottom_right; number_value :ambience }
				line(590, 600) { position_value :top_left, :bottom_right; number_value :lighting }
				line(511, 512, 521, 522) { number_value :floor }
				line(531, 532, 541, 542) { number_value :object }
				line(551, 552, 561, 562) { number_value :region }
				line(571, 572, 581, 582) { number_value :ambience }
				line(591, 592, 601, 602) { number_value :lighting }
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
				# Regions
				line(30, 31) { number_value :region }
				line(32, 33) { number_value :low_region, :high_region }
				# Effects
				line(40, 41) { number_value :effect }
				# Lighting
				line(42, 43) { number_value :lighting }
				# Ambience
				line(44, 45) { number_value :ambience }
			end


			# EFFECTS
			define_category(5) do
				# Floor/Object Changes
				line(1) { number_value :floor }
				line(4) { number_value :object }
				line(2, 5) { number_value :from, :to }
				line(3, 6) { number_value :first, :second }
				line(7, 13) { number_value :low, :high }
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
				# Random Player Movement
				line(48, 49) { number_value :region }
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
				line(114, 115) { number_value :text_filter }
				# Regions
				line(120) { number_value :region }
				line(121) { position_value :position; number_value :region }
				line(122) { number_value :threshold }
				line(123..128)
				line(130..139) { number_value :region }
				line(140, 144) { number_value :object }
				line(141, 145) { number_value :wall }
				line(142, 146) { number_value :floor }
				line(143, 147) { number_value :effect }
				line(148, 149) { position_value :position }
				# Effects
				line(150) { number_value :effect }
				line(151) { position_value :position; number_value :effect }
				line(152) { number_value :effect }
				line(153) { number_value :from, :to }
				line(154) { number_value :first, :second }
				# Lighting
				line(155) { number_value :lighting }
				line(156) { position_value :position; number_value :lighting }
				line(157) { number_value :lighting }
				line(158) { number_value :from, :to }
				line(159) { number_value :first, :second }
				# Region Configuration
				line(160..173, 178, 179) { number_value :region }
				line(174, 176) { number_value :lighting }
				line(175, 177) { number_value :ambience }
				# DS Buttons
				line(180, 181, 190, 191) { number_value :button }
				line(182, 192) { number_value :button; position_value :position }
				line(183, 193) { number_value :button, :tab }
				line(184) { number_variable :variable }
				# Region Configuration
				line(194..199) { number_value :region }
				# Emits
				line(200, 201, 203, 204) { string_value :message }
				line(202) { string_value :message; position_value :position }
				line(205) { string_value :message, :name }
				# Other Text Stuff
				line(212) { string_value :prefix }
				line(213) { number_value :floor }
				line(214) { number_value :object }
				line(215) { string_value :message }
				# Ambience
				line(225) { number_value :ambience }
				line(226) { position_value :position; number_value :ambience }
				line(227) { number_value :ambience }
				line(228) { number_value :from, :to }
				line(229) { number_value :first, :second }
				# Effects
				line(230) { number_value :low, :high }
				line(231) { number_value :distance }
				line(232..234) { position_value :position }
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
				line(280, 281, 282) { number_variable :target; string_value :source }
				line(283, 284) { number_variable :target; string_value :needle, :source }
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
				line(314, 315, 317, 318, 321..327) { number_variable :target }
				line(316) { number_value :entry_code }
				line(319) { position_variable :target }
				line(320, 330) { number_variable :target; position_value :position }
				line(350, 351) { position_variable :target }
				# Variable Relative Movement
				line(352..355) { position_variable :target; number_value :distance }
				# Map Data/Positions
				line(331, 332, 380..383) { number_variable :target; position_value :position }
				line(384) { position_variable :target; position_value :position }
				# Arrays and Misc
				line(390) { number_value :start_index, :count; number_variable :array_base; number_value :value }
				line(399)
				# Cycling
				line(400, 405, 410, 415) { number_value :one, :two, :three }
				line(401, 406, 411, 416) { number_value :one, :two, :three, :four }
				line(402, 407, 412, 417) { number_value :one, :two, :three, :four, :five }
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
				line(445) { number_value :effect, :step }
				line(446) { number_value :effect }
				line(447) { number_value :effect, :delay }
				line(448) { number_value :lighting, :step }
				line(449) { number_value :lighting }
				line(450) { number_value :lighting, :delay }
				line(451) { number_value :ambience, :step }
				line(452) { number_value :ambience }
				line(453) { number_value :ambience, :delay }
				# Move Stuff
				line(460..463, 470..473) { number_value :distance }
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
				line(714..717, 720, 721) { position_value :position }
				line(718)
				line(719) { number_value :distance }
				line(722, 723) { number_value :region }
				line(780..783) { number_value :distance }
				line(784..787) { number_value :first_distance, :second_distance }
				# PS Timed Forget
				line(880) { number_value :days, :hours, :minutes, :seconds }
				# Dialogs
				line(900, 901, 910, 911) { number_value :request; string_value :message }
				# Misc
				line(1000)
				line(2000)
				# LocalSpecies
				line(1200, 1201) { number_value :species }
				line(1202, 1203)
				line(1205) { number_value :species, :avatar }
				line(1206) { number_value :species, :avatar, :gender }
				line(1207)
				# Remaps
				line(1250) { number_value :remap, :id }
				line(1251) { number_variable :target; number_value :remap }
				# Scale
				line(3443) { number_value :percentage }
			end
		end
	end
end
