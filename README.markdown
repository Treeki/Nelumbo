Nelumbo by Treeki
=================

Nelumbo is a framework for creating Furcadia bots, written in the powerful and
flexible Ruby programming language. It uses EventMachine for networking.

## Caveats and Updated Information
I'm releasing this in November 2012, over a year after it was originally
written. While I currently don't have time to update it any further, here's
some stuff...
- The documentation is really not as good as it could be. Sorry for that :/
  Some of it also isn't 100% up to date.
- Nelumbo does not run on Ruby 1.9.3 because the "mixology" gem doesn't seem
  to be compatible with it. If you don't mind losing the plugin functionality
  you can remove *require 'mixology'* from *lib/nelumbo/base.rb*.
- Nelumbo has been powering the bot in Cypress Homes (a complex beast with
  over 6,000 lines of code, using DataMapper for database storage) for over
  a year with very few issues. This includes the DS engine and advanced
  world tracking.
- If you have any questions (and you probably will), feel free to whisper me.
- There's some interesting code in the repo history. Nelumbo used to have
  a "Core" system for dealing with timers/multiple bots, but I ditched it in
  favour of EventMachine. I admittedly can't remember why... I think I was
  having trouble getting something working. And at one point I started working
  on code that would convert DS to Ruby code, but that didn't go anywhere; I
  ended up going with a C interpreter instead (which ended up being a much
  better solution).
- I'm not planning to work on this in the near future, except for keeping it
  up-to-date with Furcadia updates.

## Current Progress
Basic bots are functional. Handling for most of the Furcadia protocol (both for
sending commands and acting on events) needs to be added. Documentation is also
needed for various classes and methods.

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

