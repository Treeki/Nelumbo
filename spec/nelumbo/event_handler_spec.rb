require 'nelumbo'

describe Nelumbo::EventHandler do
	it 'should define events' do
		cls = Class.new(Nelumbo::EventHandler) do
			define_event :test_event
			define_event :test_event2
		end
	end

	it 'should define events with arguments' do
		cls = Class.new(Nelumbo::EventHandler) do
			define_event_with_args :test_event_with_args
		end
	end

	it 'should fail when defining an event that exists' do
		expect do
			cls = Class.new(Nelumbo::EventHandler) do
				define_event :test_event
				define_event :test_event
			end
		end.to raise_error /is already defined/
	end

	describe 'when dispatching events' do
		cls = Class.new(Nelumbo::EventHandler) do
			define_event :test_event
			define_event :test_event2
			define_event_with_args :test_args
			define_event_with_args :test_cond

			on_test_event  { throw :t_e }
			on_test_event2 { throw :t_e2 }

			on_test_args { |args| throw args[:verifier] }

			on_test_cond(:num => 123, :str => 'foo', :regex => /bar/) { throw :success_all }

			on_test_cond(:num => 123)     { throw :success_num }
			on_test_cond(:str => 'foo')   { throw :success_str }
			on_test_cond(:regex => /bar/) { throw :success_regex }
			on_test_cond                  { throw :catch_all }

			attr_accessor :halt_check
			attr_accessor :halt_all_check
			attr_accessor :halt_ft_check

			define_event :halt_event
			define_event :halt_all_event
			define_event :halt_ft_event

			on_halt_event do
				@halt_check = :success
				halt
				@halt_check = :failure
			end

			on_halt_all_event do
				@halt_all_check = :success
				halt_all
				@halt_all_check = :failure
			end

			on_halt_all_event do
				@halt_all_check = :failure
			end

			on_halt_ft_event do
				halt
			end

			on_halt_ft_event do
				@halt_ft_check = :success
			end
		end

		eh = cls.new

		it 'should dispatch basic events correctly' do
			expect { eh.dispatch_event :test_event }.to  throw_symbol :t_e
			expect { eh.dispatch_event :test_event2 }.to throw_symbol :t_e2

			expect { eh.dispatch_event :test_event }.to_not throw_symbol :t_e2
		end

		it 'should pass on the arguments' do
			expect { eh.dispatch_event :test_args, {:verifier => :blah} }.to throw_symbol :blah
		end

		it 'should handle argument conditions properly' do
			expect { eh.dispatch_event :test_cond, :num => 123 }.to throw_symbol :success_num
			expect { eh.dispatch_event :test_cond, :num => 456 }.to_not throw_symbol :success_num

			expect { eh.dispatch_event :test_cond, :str => 'foo' }.to throw_symbol :success_str
			expect { eh.dispatch_event :test_cond, :str => 'not foo' }.to_not throw_symbol :success_str

			expect { eh.dispatch_event :test_cond, :regex => 'at a bar' }.to throw_symbol :success_regex
			expect { eh.dispatch_event :test_cond, :regex => 'nowhere' }.to_not throw_symbol :success_regex

			expect { eh.dispatch_event :test_cond, :num => 600, :str => 'nope', :regex => 'nothing' }.to throw_symbol :catch_all
		end

		it 'should handle multiple conditions properly' do
			expect { eh.dispatch_event :test_cond, :num => 123, :str => 'foo', :regex => 'bar' }.to throw_symbol :success_all
			expect { eh.dispatch_event :test_cond, :num => 123, :str => 'nope', :regex => 'bar' }.to_not throw_symbol :success_all
		end

		it 'should handle halting a responder properly' do
			eh.dispatch_event :halt_event
			eh.halt_check.should == :success
		end

		it 'should handle halting all responders properly' do
			eh.dispatch_event :halt_all_event
			eh.halt_all_check.should == :success
		end

		it 'should fall through to the next matching responder when a responder is halted' do
			eh.dispatch_event :halt_ft_event
			eh.halt_ft_check.should == :success
		end
	end

	describe 'when using subclasses' do
		cls_a = Class.new(Nelumbo::EventHandler) do
			define_event :base_event_a
			define_event :event_a

			on_base_event_a { throw :base_a_worked }
		end

		cls_b = Class.new(cls_a) do
			define_event :event_b

			on_event_a { throw :a_worked }
			on_event_b { throw :b_worked }
		end

		b = cls_b.new

		it 'should dispatch events to superclasses' do
			expect { b.dispatch_event :base_event_a }.to throw_symbol :base_a_worked
			expect { b.dispatch_event :event_a }.to throw_symbol :a_worked
		end

		it 'should dispatch events to the subclass correctly' do
			expect { b.dispatch_event :event_b }.to throw_symbol :b_worked
		end
	end
end

