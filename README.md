exMARS  -- Exhaust Memory Array Redcode Simulator

exMARS is a redcode simulator, just like Exhaust and pMARS. In fact, I
have shamelessly taken sourcecode from both  [pMARS](http://www.koth.org/pmars/), [exhaust](http://www.cs.helsinki.fi/u/jpihlaja/exhaust/exhaust.html), some
ideas from [qmars](https://github.com/martinus/qmars) plus further optimizations, a higher level
interface for [Ruby](http://www.ruby-lang.org/en/), and shaked everything very well. 

The resulting programm has the following main features:

* Uses the parser from pMARS, so no previous parsing is neccessary.
* Speed: 50% faster than pmars on a pentiumIII, and about twice as fast
  on a pentium 4 on average than pmars (using gcc 3.3.1).
* Rewritten the C code in a more object oriented way, which allows
  different Mars'es at the same time in the same program. 
* Ruby interface: finally a really fast mars can be used in a high level
  programming language. (see test.rb for an example usage)


Have fun!

Martin
