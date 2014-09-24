InfectC
=========

##A.K.A. Ebola Simulator

##Version 0.1

![alt text](http://fc07.deviantart.net/fs70/i/2014/238/0/2/ebola_chan_pony_by_kaboderp_sketchy-d7wtbj5.jpg "Thank you, Ebola-chan!")

Epidemic simulator rewritten from Perl to C. The original idea, code, ruleset and gameplay elements were all imagined by [Ziddykins](https://github.com/Ziddykins/Infect)

Balance changes provided by [ilovecock](https://github.com/ilovecock/Infect).

##Screenshots to be added

![alt text](http://imgur.com/abc.jpg "Add screenshot here")

![alt text](http://imgur.com/abc.jpg "Add screenshot here") 

##Play

Draft Commands:

- ```-h``` or ```--help```: print the help
- ```-v``` or ```--version```: print the version information
- ```--slow```: refresh the screen every 10 days
- ```--fast```: refresh the screen every 5 days
- ```--fastest```: refresh the screen every day
- ```--steps```: refresh step-by-step by pressing ```n``` on the keyboard
- ```-m``` or ```--map```: specify the path to a custom map to load it. Incompatible with the following flags.
- ```-x``` and/or ```-y```: specify the dimensions of the map
- ```-d```, ```-i```, ```-n```,```-s```, ```-w```: specify the number of doctors, infected, nurses, soldiers and/or wood

The default arguments are X = 79, Y = 20, --fast. If you don't specify the x/y dimensions, 16 doctors, 8 infected, 32 soldiers and 80 nurses are spread randomly on the map and you get 790 wood logs. They scale with the dimensions automatically.

Q quits the simulation.

##Dependencies

- ncurses
- make

##Installation

- Run ```make``` to get a release build or ```make DEBUG=1``` to get a debug build, which logs to infect.log

##Known issues

- None at this time.

##To do:

- Update this README.md

