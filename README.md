# Source SDK 2013 BDS Base
<img src="https://github.com/BitlDevelopmentStudios/source-sdk-2013-bds-base/blob/master/bdsbase.png" alt="Logo" width="450" height="450">

This is a Source SDK 2013 fork made for the purpose of giving a reliable mod base for currently updated and future Bitl Development Studio (BDS) mod projects.
This was based off the TF2/64-bit SDK release, and will be updated as the SDK updates.

This will feature sources for some currently updated and future Bitl Development Studio (BDS) mod projects. 
This base is specific to multiplayer mod projects.

## Projects using this base:
- SURVIVOR II (Based on HL2DM, Active Development)
- Quiver Fortress (Based on TF2, Shelved)

## Features:
- Restored game project generator scripts. No more having to generate every project!
- Implements various pull requests (200+) from the master repo, allowing for a stable and reliable mod base.
- NPC NextBot sensing support from FIREFIGHT RELOADED with the BDSBASE_NPC preprocessor definition.
- Integrated Python binaries (on Windows) for more simple buiding.
- Implemented Discord RPC support with the BDSBASE_DISCORD preprocessor definition.
- Full support of the Half-Life 2 Survivor animation set for all mods.
- reset.bat file in each mod folder, used for cleaning up config/temporary files for easy mod distribution.
- Implemented bhopping functionality that can be enabled or disabled by server owners in TF2 and HL2DM.
- Multiplayer NPC support with the BDSBASE_NPC preprocessor definition.
- Various optional preprocessor definitions for changing TF2's behavior.
- Menu music in map backgrounds with adjustable volume and mix volume.
- Server Admin tools support (based off #948) with extendable module-based command interface.
- Animated Avatars support (based off #1380)
- Improved AI Bot behavior (based off multiple pull requests)
- Added randomized loadouts to AI bots that allow bots to spawn with a variety of weapons. (TF2 only)

## Options/Preprocessor Definitions
These options are meant to be added to the VPC files (client and server) of the mod you wish to modify.
These are optional and don't impact the usability of the core fork if they're not enabled.

BDSBASE_NPC: 
- Games: All
- Enables multiplayer NPC support as well as NextBot visibility fixes for NPCs.

BDSBASE_DISCORD: 
- Games: All
- Enables Discord integration. Note that this is a Windows only feature due to the instability of Discord clients on Linux, and the Discord RPC library will be included with your solution. 
- Mod authors can change the DiscordAppId parameter in their gameinfo.txt file. 
- Mods can also use the DiscordAllowMapIcons option to allow map specific icons. 
- Mod icons must be named "ModImage" and map icons must be named the map name.

BDSBASE_USE_LOCAL_SCHEMA
- Games: TF2
- Forces the game to use the local items_game.txt instead of the one from the GC.

BDSBASE_CUSTOM_SCHEMA
- Games: TF2
- Enables the creation of a items_custom.txt file that allows the addition and modification of items without needing to modify the items_game.txt file.

BDSBASE_LEGACY_MAINMENU
- Games: TF2
- Enables the old main menu from older versions of TF2.

BDSBASE_CURATED_ITEMS
- Games: TF2
- NOTE: You don't need to enable this to blacklist items. To blacklist items, simply add "blacklist" 1 in its item definition.
- NOTE: If you'd like to prevent bots from using some items, even if they are whitelisted, add "usable_by_bots" 0 to the item's definition. Note that this will get ignored in MvM.
- Prevents non-stock items from showing up in the loadout panel and loads whitelisted items for a curated item selection. 
- To add an item to the curated item list, simply add "whitelist" 1 in its item definition. 
- You may re-enable cosmetics and weapon stranges, reskins, etc by using BDSBASE_CURATED_ITEMS_ALLOWCOSMETICS. Items that represent reskins must have "reskin" set to 1 in your desired item schema definitions. 
- You may re-enable only weapon stranges, reskins, etc by using BDSBASE_CURATED_ITEMS_ALLOWCOSMETICWEAPONS. Items that represent reskins must have "reskin" set to 1 in your desired item schema definitions. 
- Using this with BDSBASE_CUSTOM_SCHEMA would also allow custom items to be allowed with this option. If you only need BDSBASE_CUSTOM_SCHEMA for custom attributes, use BDSBASE_CURATED_ITEMS_DISABLE_CUSTOMITEMS.

BDSBASE_ECON_LOADOUT_ONLY
- Games: TF2
- Disables access to the backpack or other econ panels.

BDSBASE_LEGACY_VIEWMODELS
- Games: TF2
- All stock items will use the legacy view models. This will only apply to the "base" default items, allowing botkillers, skins, etc to function properly. 
- Strange versions of stock weapons won't have these legacy view models, as adding them would break the other weapons listed as "Upgradeable".
- Some stock items don't get the viewmodel treatment, due to bugs, visual problems, or other factors. These include the Pistol, the Shotgun, the Fists, the PDAs, the Sapper, and the Toolbox. 
- The Spy's Disguise Kit and the Invis Watch both already use view models, so they aren't affected by this definition.

Explanations for the cut weapon models:

- The Pistol has a problem where it rapidly plays the draw animation when switching classes. The gunslinger is also why this is. I made a c_pistol model used for the base item version that replicates the v_model version's lighting, so it doesn't stand out.
- The Shotgun models work, however all of them have an annoying texture seam on the top of the gun and the viewmodels don't add as much of a visual improvment in comparison to the c_model, so I just used the c_model, which fixes the seam issue and has comprable visual fidelity. Also, the gunslinger. I'll do a similar thing I did with the Pistol, where there will be a model used for the baseitem that uses the same lighting as the viewmodel.
- The PDA's and Toolbox's models work, but they were cut because of the Gunslinger.
- The Sapper's model kept switching to the Toolbox's in most instances, so it was changed to the c_model.

The Gunslinger is the reason for some of these cuts since it is a seperate bodygroup on the c_model arms, making the idea of using any of the legacy viewmodels on Engineer (except for the wrench) unviable

BDSBASE_ACHIEVEMENT_NOTIFICATIONS
- Games: All
- This mode enables the Xbox 360-styled achievement notifications, and enables the achievement_earned event for in-game effects. (i.e. TF2's achievement announcements)
- Users can easily change the look of the panels with the cl_achievements_theme command.
- This allows users to easily see unlocked achievements and achievement progress.

BDSBASE_TEMP_FIXDYNAMICMODELS
- Games: All
- This enables Pull Request #1483 (https://github.com/ValveSoftware/source-sdk-2013/pull/1483) by default which fixes issues with dynamic models in listen servers.
- This is a temporary workaround and is not recomended to be enabled with future versions of the engine.
- If this is disabled, use the command line variable -dynamicmodelsfix to enable the fix.

BDSBASE_DISABLE_ITEM_DROP_PANEL
- Games: TF2
- This disables the panel shown for unacknowledged items in live TF2.

## Credits:
- TheBetaM for the custom schema code. (https://github.com/TheBetaM/tf-solo)
- rafradek for some bot AI changes (sigsegv-mvm) (https://github.com/rafradek/sigsegv-mvm)
- Mastercoms for the inventory code rewrite (https://github.com/mastercomfig/tc2/commit/cc256a113abe9eb0530cc45d07aa4e00b187d5a8)
- Missing Killicons Pack by NeoDement
- Ultimate Visual Fix Pack by agrastiOs, Nonhuman, N-Cognito, Whurr, PieSavvy, JarateKing, and FlaminSarge
- Fixed Spy's View Model Sleev V2 by Hecates and H.Gaspar (https://gamebanana.com/mods/193390)
- The Valve Developer Community for the following articles:
https://developer.valvesoftware.com/wiki/Detail_props/Aspect_ratio_fix
https://developer.valvesoftware.com/wiki/General_SDK_Snippets_%2526_Fixes

## Setup:
Read Autumn/Misyl's setup guide at README_FROG.md for detailed setup.
You may also read a more detailed guide here:
https://developer.valvesoftware.com/wiki/Source_SDK_2013