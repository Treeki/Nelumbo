Nelumbo by Treeki
=================

Nelumbo is a framework for creating Furcadia bots, written in the powerful and
flexible Ruby programming language. It uses EventMachine for networking.

## Current Progress
Basic bots are functional. Handling for most of the Furcadia protocol (both for
sending commands and acting on events) needs to be added. Documentation is also
needed for various classes and methods.

One little quirk of the system right now is that for the advanced bot manager
features to work properly, your bot code should **all** be located in a module
named after the bot's .rb file. Also, the class should be named Bot.
For example, if the file is named test\_bot.rb, the bot class should be
TestBot::Bot. Putting everything into that module will make reloading work
properly!

Eventually, I'll probably also need to make the Cores and the Bot/BaseBot
classes thread-safe. That can come later though.

Documentation is really required. I'll probably use YARD for this.

I need more specs too. Stuff like protocols and DS engine are really hard to
write specs for, though... I can probably do them for the DS parser and the
plugin system. But I'm lazy.

## Key Features
- Simple Sinatra-style DSL for handling and responding to events
- Automatically includes ActiveSupport
- EventMachine used for connection handling
- Integrated - but optional - DS engine for full dream tracking (Requires
  an extension module)
- Clean plugin system using mixins and the Mixology gem

## Planned Features
- Daemon version that can run multiple bots
- Web interface
- Simple GTK interface that runs one bot at a time for non-technical users
- DS-like scripting language for non-technical users

## Example: A Simple Bot (simple\_bot.rb)
    require 'nelumbo'

    class SimpleBot < Nelumbo::Bot
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

    SimpleBot.set username: ARGV.first, password: ARGV.last
    Nelumbo::run_simply SimpleBot

