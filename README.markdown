Nelumbo by Treeki
=================

Nelumbo is a framework for creating Furcadia bots, written in the powerful and
extensible Ruby programming language.

## Current Progress
Basic bots are functional. Timers still need to be added, as does handling for
most of the Furcadia protocol (both for sending commands and acting on events).
Documentation is also needed for various classes and methods.

One little quirk of the system right now is that for the advanced bot manager
features to work properly, your bot code should **all** be located in a module
named after the bot's .rb file. Also, the class should be named Bot.
For example, if the file is named test_bot.rb, the bot class should be
TestBot::Bot. Putting everything into that module will make reloading work
properly!

I need to add a way for the Core to be controlled, so I can write the
web interface, since integrating it into SelectCore seems clumsy.

Eventually, I'll probably also need to make the Cores and the Bot/BaseBot
classes thread-safe. That can come later though.

## Key Features
- Simple Sinatra-style DSL for handling and responding to events
- Automatically includes ActiveSupport
- Modular: sockets/timers are managed by a "Core" and can be swapped easily

## Planned Features
- Daemon version that can run multiple bots
- Web interface
- Simple GTK interface that runs one bot at a time for non-technical users
- DS-like scripting language for non-technical users
- Plugins

## Example: A Simple Bot (simple_bot.rb)
    require 'nelumbo'

    module SimpleBot; end

    class SimpleBot::Bot < Nelumbo::Bot
      set color_code: 't::)5,&(@-&$%#'
      set description: 'A simple bot powered by Nelumbo.'

      on_speech text: /what((')?s| is) the time/i do
        say "#{data[:name]}, the time is #{Time.now}."
      end

      on_join_request { summon data[:name] }

      on_whisper text: 'Who are you?' do
        whisper_back "I am a bot!"
      end
    end

