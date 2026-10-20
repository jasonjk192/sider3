# Introduction

Sider 3 is a companion program for Pro Evolution Soccer 2017.

# How to build

You need Microsoft Visual C++ compiler.
Visual Studio 2010 Express Edition is assumed in the setenv.cmd file, but you
should be able to use other versions of Visual Studio too.

Build is done from command-line.
Start cmd.exe, then switch to source directory, then:

    setenv.cmd
    nmake


# How to use

- Download [v3.5.1](https://github.com/pes-modding/sider3/releases/tag/v3.5.1) or use the one from Nexusmods.
Alternatively you can use SiderLite.
- Install whichever choice you prefer.
- Download [latest version](https://github.com/jasonjk192/sider3/releases) from this forked repo.
- Replace sider.exe and sider.dll in your chosen version.

if not using SiderLite, you can also add these commands into sider.ini to autorun PES17 when you start sider3:

    game.autorun = 1
    game.exe.path = "..\PES2017.exe" ; Relative or absolute path to your .exe

