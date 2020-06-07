# `modules`
### Discord
For Discord, the **[Game SDK](https://discordapp.com/developers/docs/game-sdk/sdk-starter-guide)** is required. <br>
Copy the C++ headers and sources from the `cpp` folder and `discord_game_sdk.dll.lib` as well as `discord_game_sdk.dll` from the `lib`/<Architecture> folder into the `discord` directory.
You can take either the `x86` or `x86_64` version, both will work as long as the correct project architecture is chosen.

### iTunes
In order to communicate with iTunes, the COM interface is necessary. <br>
You can download iTunes' COM interface on Apple's **[Developer Downloads](https://developer.apple.com/download/more/)** website. <br>
Copy `iTunesCOMInterface_i.c` and `iTunesCOMInterface.h` into the `itunes` directory.