# Battleship

[![CI](https://github.com/ayoubnasr/BattleShip/actions/workflows/ci.yml/badge.svg)](https://github.com/ayoubnasr/BattleShip/actions/workflows/ci.yml)

A terminal Battleship game written in C. You can play against a friend or against a bot. The hard bot hunts for your ships with a live
probability map of your grid, and adjusts how much risk it takes depending on how well you are playing.

Built for **CMPS 270 (Software Construction) at the American University of Beirut, Fall 2024**.

<p align="center"><img src="docs/screenshot.png" alt="The hard bot hunting down a carrier: hits in red, misses as O, and the bot announcing its last shot" width="360"></p>

## Features

- **Two modes:** PvP (two players, one keyboard) and PvE (you against the bot).
- **Two bots:** an easy bot that fires at random, and a hard bot that uses probabilities.
- **Special weapons:** radar sweeps, smoke screens, artillery and torpedoes, each with its own unlock rule.
- **Two difficulties:** on hard, your misses are not marked on the grid.
- **Scalable:** the grid size is one macro, and the coordinate system grows with it (`A`…`Z`, then `AA`, `AB`…).

## Getting started

You need a C compiler and `make`:

| Platform | What to install |
| --- | --- |
| Linux | `gcc` and `make` (eg. `sudo apt install build-essential`) |
| macOS | Xcode command line tools (`xcode-select --install`) |
| Windows | [MinGW-w64](https://www.mingw-w64.org/) (gcc + `mingw32-make`), or WSL |

```sh
git clone https://github.com/ayoubnasr/BattleShip.git
cd BattleShip
make            # builds bin/battleship (bin/battleship.exe on Windows)
make run        # builds and starts the game
```

On Windows with MinGW, build with `mingw32-make` and start the game with `bin\battleship.exe`. Run it in a terminal that understands ANSI
colors (Windows Terminal, or any recent console).

**No compiler?** Every CI run builds a Windows executable: open the latest successful run on the
[Actions tab](https://github.com/ayoubnasr/BattleShip/actions/workflows/ci.yml) and download the `battleship-windows` artifact.

## How to play

1. Type `start`, choose the difficulty (`easy` or `hard`) and the mode (`PVP` or `PVE`).
2. Enter your name and place your four ships: give the first cell (eg. `B3`) and the orientation (`H` or `V`).
3. In PvE, pick the bot's difficulty (`easy` or `hard`). A random player goes first.

| Ship | Length |
| --- | --- |
| Carrier | 5 |
| Battleship | 4 |
| Destroyer | 3 |
| Submarine | 2 |

Coordinates are a column letter and a row number: `A1` is the top-left cell and `J10` the bottom-right (on the default 10x10 grid).
Lowercase works too.

### Commands

| Command | What it does | When you can use it |
| --- | --- | --- |
| `fire B4` | Fires at one cell. | Always. |
| `radar B4` | Tells you if any enemy ship is in the 2x2 area starting at `B4`. | 3 times per game. |
| `smoke B4` | Hides the 2x2 area starting at `B4` on your grid from enemy radar. | Once for each ship you have sunk. |
| `artillery B4` | Fires at the whole 2x2 area starting at `B4`. | Only on the turn right after you sink a ship. |
| `torpedo C` / `torpedo 7` | Fires at a whole column (letter) or row (number). | Only on the turn right after you sink a ship, once you have sunk 3. |
| `next` | Skips your turn. | Always. |
| `quit` | Exits the game. | At any prompt that asks you for something. |

On the grid, `~` is water, `x` is a hit and `O` is a miss (misses are only shown on **easy**). The first player to sink all four of the
opponent's ships wins.

## The bot

The hard bot is the heart of the project. The full design is in the
[presentation](docs/presentation.pdf) and the [bot design document](docs/bot-design.docx), and the long comment at the top of
[`src/Bot.c`](src/Bot.c) walks through the thinking behind it. In short:

1. **Probability grid** ([`CalcProbs.c`](src/CalcProbs.c)). Every cell of your grid gets a score: the number of ways the remaining ships
   could pass through it, worked out with a formula, not loops. Hits raise the score of the cells in line with them and misses lower it.
   When a ship sinks, the whole grid is recalculated without it.
2. **Probability regions** ([`Player.c`](src/Player.c), [`Bot.c`](src/Bot.c)). Always shooting the highest-scoring cell would make the bot
   predictable. So the grid is split into small regions, and each region keeps its cells in a **binomial heap**, ordered by probability.
   The regions are sorted into **high, average and low** probability lists. The bot picks a random region from a list, then takes that
   region's best cell. A hash set from region to heap and list node means that updating a region after a shot and moving it to another
   list both take constant time.
3. **Risk** ([`Bot.c`](src/Bot.c), `[Risk]` section). The bot watches your accuracy, your success rate over the last few turns, and how
   many ships each side has sunk. Normally it takes turns picking from the high, average and low lists. When you are doing badly, it
   takes more risks and sometimes explores elsewhere in the middle of sinking a ship. When you are doing well, it sticks to the highest
   probability regions, and when you are doing very well it fires at the single hottest cell.
4. **The brain** ([`Bot.c`](src/Bot.c)). A stack of tasks (a function pointer plus its arguments) that carries plans across turns: after a
   hit, the cells around it go on top of the stack and get fired at on the next turns until the ship sinks.
5. **Weapons and placement.** The hard bot uses its artillery and torpedo whenever the rules allow, aimed at the area with the highest
   total probability. It places at least one of its own ships where it is least likely to be found.

The easy bot fires at random, but it never fires at the same cell twice and it skips cells where no remaining ship could fit.

### Tuning

Most of the game and the bot's intelligence can be changed with macros:

| Macro | File | Effect |
| --- | --- | --- |
| `GRIDSIZE` | [`include/defs.h`](include/defs.h) | Size of the (square) grid. |
| `PROB_REGION_WIDTH` / `PROB_REGION_HEIGHT` | [`include/Player.h`](include/Player.h) | Region size: the "resolution" of the bot's statistics. Smaller regions mean a sharper bot. |
| `HIGHPROB_BASE` / `AVGPROB_BASE` / `LOWPROB_BASE` | [`include/Player.h`](include/Player.h) | Probability needed for a region to be in the high, average or low list. |
| `RISK_*`, `LOWRISK_EXPLORE_CHANCE` | [`include/Player.h`](include/Player.h) | How quickly the bot feels threatened, and how adventurous it gets at low risk. |

## Tests

```sh
make test
```

This runs the unit tests in [`tests/`](tests). They need nothing but the compiler and cover:
- coordinates and input parsing
- the linked list and the binomial heap
- ship placement and every attack
- the probability grid and the region system (its consistency is checked after every shot)
- both bots playing complete games

CI runs them on Linux and macOS on every push.

## Project structure

```
.
├── src/                 game source
│   ├── Driver.c         main(), menus, turns and everything printed on screen
│   ├── Player.c         player setup, grid display, probability regions
│   ├── Bot.c            the bots: targeting, task stack, weapons, risk, ship placement
│   ├── CalcProbs.c      probability formulas
│   ├── Attacks.c        fire, radar, smoke screen, artillery, torpedo
│   ├── ShipPlacement.c  placing ships on a grid
│   ├── BinomialHeap.c   generic binomial heap
│   ├── D_LinkedList.c   generic doubly linked list
│   ├── coordslib.c      coordinate system (eg. "B4" -> row 3, column 1)
│   ├── InputLib.c       safe input reading and parsing
│   ├── UITools.c        colors, centered printing, console size
│   ├── ShortcutFuncs.c  small string helpers
│   └── defs.c
├── include/             headers (constants and settings are in defs.h and Player.h)
├── tests/               unit tests (make test)
├── docs/                presentation, design documents, backlog and test plan
└── Makefile
```

## Authors

- Ayoub Nasr
- Ali Haddam
- Fadi Bayan
- Hussein Ghaddaf
