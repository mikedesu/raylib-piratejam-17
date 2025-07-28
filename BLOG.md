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

*7/20/2025: 7:32 pm*

- want to implement coins and exp.
- coins will be collected to buy power ups when u run into a merchant.
  - power ups will include things like:
      - making the sword bigger
      - making the sword do more damage
      - making the sword have more durability
      - making the player have more controlled or faster movement
      - making the player's hitbox smaller
      - spawning a shield that sits in front and takes a hit
- exp will be used to level up.
    - some power ups may be unavailable at lower levels 
- spawning of enemy initial position should be a bit fixed on the x so as to be predictable timing-wise

----------

*7/25/2025: 3:34 pm*

- greetings stream watchers!

- we need an HUD. to start with:
    - hp
    - durability
    - speed
    - level
    - coins
    - kills
- up and down directions so the sword can also aim up and down
- enemies to spawn in from the left as well
- enemies with greater hp
- power up that increases attack power
    - will require an ATTACK component
- power up that acts as a bomb that wipes the screen
    - will require a BOMBS component
- i want to track combos and display them in a cool way



7/27/2025

- need to make dwarfs 5-laned
- need bats coming in from top (DONE)
  - need sword up/down first (DONE)
- need gameover stats display (DONE)
    - enemies killed/missed/percentages
    - coins collected/missed/spent
    - etc

7/28/2025

- WEB BUILD!!!
- DESIGN DOC!!!
- boss orc / big orc with lots of hp
- diagonal swords?
- sword attack power power up
- finish tutorial screen 
- "you died" screen before game over screen





