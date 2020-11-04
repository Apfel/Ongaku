# Ongaku for Windows
A port of **[Ongaku](https://github.com/spotlightishere/Ongaku)** for Windows.  
Ongaku allows you to share what you're listening to and/or watching on iTunes in Discord.  
Requires **[Discord](https://discordapp.com)** and **[iTunes](https://www.apple.com/itunes/)**.  

## Usage
Just run `Ongaku.exe`!  
To quit, click the Ongaku icon in the System Tray on your Taskbar.

## Build
Before even configuring anything, you must install all required third-party dependencies inside the `ThirdParty` directory.  
Create it and any inside a cloned version of the repository, if necessary.

| Dependency       | URL | Folder Name inside the `ThirdParty` folder | Files to copy / Configuration |
|---|---|---|---|
| Discord RPC | [discord-rpc Repository](https://github.com/discord/discord-rpc) | `Discord` | Configurations to install: Debug, Release (see below) |
| iTunes COM SDK   | [Apple Developer - Downloads](https://developer.apple.com/download/more/) | `iTunes` | Files to copy: `iTunes_COM_<version>/iTunesCOMInterface_i.c`, `iTunes_COM_<version>/iTunesCOMInterface.h` |

None of the files need to be inside a subdirectory within their respective module directory.  
The `Files to copy` section refers to relative paths within the given ZIP archive.

Discord RPC uses CMake as its build system; since Windows' Visual Studio generator (and others) support multiple configurations, Ongaku is set up to work with a `Debug` and a `Release` configuration.  

Installing Discord RPC is fairly straightforward:
1. Download a copy of the repository (preferably via `git`).
1. Open the command line inside the directory.
1. Generate the configurations with the usual `cmake -B build`.
1. Build it via `cmake --build build --config <config>`. Build it for **both** types (see above).
1. Install it into `<root dir>/ThirdParty/Discord/<config>`: `cmake --install build --prefix /path/to/a/clone/of/this/repository/ThirdParty/Discord/<config>`. Again, for **both** configuration types.

Now you've set up the third party dependencies.  
Building Ongaku is a little more complex, however; you need to, due to limitations of my own abilities, set it during both generation and building:
```sh
cmake -B build -DCMAKE_BUILD_TYPE=<config>  # Generate
cmake --build build --config <config>       # Build
```

You can, if successfully built, find a binary (and all required files) within the directory of the same name as your set configuration, inside the `build` directory.

#### **Developer notes**
I used version **9.1.0.80** of the iTunes COM SDK.  
I suspect that iTunes won't get a massive update on Windows in the foreseeable future, but just for the record, there you go.
