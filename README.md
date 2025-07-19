# README.md

*7/18/2025: 01:15 am*

This is essentially the "design doc".
Going in, I am thinking something in-between a shmup and an infinite runner.
I have a dude on the screen, moving around, appropriately zoomed-in.
I have a sword appearing as well. 

Because the theme is "only one":

- Only one source file. Hardcore, but the build speed is so fast!
- Only one sword. As you kill things and collect powerups, your sword will grow in power.
- Only one life. I want to be rather brutal/punishing if you mess-up.
- Only one hitpoint. If you get hit, it is gameover.
  - However, this will encourage "highscore"-style competition.
  - Who can get the furthest?
  - Who can kill the most?
  - Who can level-up their sword the furthest?

*7/18/2025: 02:05 am*

- Orcs spawning with velocity. 
- Need collision soon.
- Need off-screen removal.
- Need kill count.
- Need parallax scrolling.
- current LOC: 503 including whitespace

*7/19/2025: 12:31pm*

- pushed current code and readme to github
- basic sound effects are in
- collision is in
- enemy deaths are in
- player getting hit is in

**Inspiration for the game comes from Radiant Silvergun as well as Gradius III***

So we are going to need:

- Powerups
    - Player
    - Sword

At the moment, when an orc runs into your sword while you have it out, the orc dies, and your sword is forced to re-sheath.
Each orc only has 1 HP, but I want to experiment with some ideas:

- Enemies with varying HP
- "Sword durability" that allows for more sword-enemy collisions before being sheathed




