# catch_error

**Quick links:** [documentation](https://yal.cc/docs/gm/catch_error/)
· [itch.io](https://yellowafterlife.itch.io/gamemaker-catch-error)
· [GM Marketplace](https://marketplace.yoyogames.com/assets/7921/_)
· [license](LICENSE)  
**Versions**: GameMaker: Studio, GameMaker Studio 2 (<2.3)  
**Platforms**: Windows, Windows (YYC)

This extension brought error handling to GameMaker games prior to its official introduction in GMS2.3.

`catch_error` intercepts error dialogs, allowing to choose how they will be shown to user
(and whether they will be shown at all).

It also features a slightly-risky option to ignore errors altogether,
allowing to display/process errors in-game.

Combined with
[Sentry](https://marketplace.yoyogames.com/assets/7917/gmsentry),
it can be used to implement automatic error reporting.

## What's interesting here

- WinAPI hooking (`catch_error.cpp`) is used to intercept the error dialogs.
- Instruction patching (`magic_patcher.h`) is used to make the game not quit after fatal errors.  
  You'll regret 

## Known issues

- Not every function exits cleanly after throwing an error in old versions of GameMaker,
  so ignoring fatal errors may lead to the game hard crashing afterwards.  
  Fixing unforeseeable engine implementation oversights is slightly out of scope for this extension.
 
## Things you can do with this:

- Show custom messages for GML errors, or hide them entirely
- Relaunch the game after a fatal error occurs
- Save error messages on exit for viewing or later use
- Pass error messages to external applications
- Automatically collect errors  from players
  (don't forget to write a privacy policy!)
- Use the system as makeshift try-catch

## Preparing to build

Needless to say, this requires basic familiarity with Visual Studio, Command Prompt/PowerShell, and Windows in general.

### Setting up GmxGen

1. [Install Haxe](https://haxe.org/download/) (make sure to install Neko VM!)
2. [Download the source code](https://github.com/YAL-GameMaker-Tools/GmxGen/archive/refs/heads/master.zip) 
(or [check out the git repository](https://github.com/YAL-GameMaker-Tools/GmxGen))
3. Compile the program: `haxe build.hxml`
4. Create an executable: `nekotools boot bin/GmxGen.n`
5. Copy `bin/GmxGen.exe` to a folder your PATH (e.g. to Haxe directory )

### Setting up GmlCppExtFuncs

1. (you should still have Haxe and Neko VM installed)
2. [Download the source code](https://github.com/YAL-GameMaker-Tools/GmlCppExtFuncs/archive/refs/heads/master.zip) 
(or [check out the git repository](https://github.com/YAL-GameMaker-Tools/GmlCppExtFuncs))
3. Compile the program: `haxe build.hxml`
4. Create an executable: `nekotools boot bin/GmlCppExtFuncs.n`
5. Copy `bin/GmlCppExtFuncs.exe` to a folder your PATH (e.g. to Haxe directory )

## Building

Open the `.sln` in Visual Studio (VS2019 was used as of writing this), compile for x86 - Release.

If you have correctly set up `GmxGen` and `GmlCppExtFuncs`,
the project will generate the `autogen.gml` files for GML<->C++ interop during pre-build
and will copy and [re-]link files during post-build.

## Meta

**Author:** [YellowAfterlife](https://github.com/YellowAfterlife)  
**License:** Custom license (see `LICENSE`)