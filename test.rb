require "Mars"

# create a new Mars with default settings, for two warriors
m = Mars.standard(2)

# load all warriors from directory 'warrior'
w1 = m.parse("warriors/94nop/behemot.red")
w2 = m.parse("warriors/94nop/reepicheep.red")

m.rounds = 200
puts "behemot vs. reepicheep"
puts m.fight(w1, w2).results_str

