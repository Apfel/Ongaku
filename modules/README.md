# `modules`
### Discord
For Discord, the **[Game SDK](https://discordapp.com/developers/docs/game-sdk/sdk-starter-guide)** is required.  
Copy the C++ headers and sources from the `cpp` folder and `discord_game_sdk.dll.lib` as well as `discord_game_sdk.dll` from the `lib`/<Architecture> folder into the `discord` directory.
You should take the `x86_64` version, though `x86` should work fine, too.

### iTunes
In order to communicate with iTunes, the COM interface is necessary.  
You can download iTunes' COM interface on Apple's **[Developer Downloads](https://developer.apple.com/download/more/)** website.  
Copy `iTunesCOMInterface_i.c` and `iTunesCOMInterface.h` into the `itunes` directory.