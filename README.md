# NFS Heat Raise Via CTS
<img width="4000" height="2250" alt="HEAT RAISE VIA CTS thumbnail (1)" src="https://github.com/user-attachments/assets/3f494178-e257-430e-9ad2-f7b6b563694c" />

## Installation
1. Download the latest version of the [Ultimate ASI Loader](https://github.com/ThirteenAG/Ultimate-ASI-Loader).
2. Copy or move `dinput8.dll` into the root folder of NFS.
3. Download this mod from the [releases page](https://github.com/Kevin4e/NFSHeatRaiseViaCTS/releases).
4. Copy or move `NFSMWHeatRaiseViaCTS.asi` or `NFSCHeatRaiseViaCTS.asi` into the game's `scripts` folder.

## Configuration
The script will now load successfully, using the default CTS Thresholds set by me.  
You can customize these thresholds by editing the corresponding .ini file:

- Generic (works for both Most Wanted 2005 and Carbon) → `NFSHeatRaiseViaCTSThresholds.ini`.

- Game-specific:
  * For Most Wanted 2005 → `NFSMWHeatRaiseViaCTSThresholds.ini`;
  * For Carbon → `NFSCHeatRaiseViaCTSThresholds.ini`.

## Notes
- Each threshold must fall within the range **0 and (2^32) - 1**.  
  If it doesn't, it will overflow and possibly mess up the order.

- If a CTS threshold is not consistent (e.g. 1000->2000-><u>500</u>) it will determine either of the three behaviors, depending on the calculation approach chosen:

  - **Progressive**: read as 'Start from the lowest. If CTS meets the next heat’s threshold, move up; otherwise, stay', having this situation:

        CtsHeat2: 10000
        CtsHeat3: 5000
        CTS = 12000 → Heat 2 threshold is satisfied → immediately also satisfies Heat 3 (5000) → ends up at Heat 3.
    
  - **Absolute**: read as 'If CTS reaches ANY threshold for a heat level, lower heat levels are ignored.', having this situation:

        CtsHeat8: 7000
        CtsHeat9: 5000
        CTS = 6000 → Heat 9 threshold is satisfied → Heat 8 is ignored → remains at Heat 9.

  - **Cumulative**: read as 'If CTS reaches ANY threshold for a heat level, switch to it, regardless of the order', having this situation:

        CtsHeat6: 3000
        CtsHeat7: 1500
        CTS = 2000 → Heat 7 threshold is satisfied. Eventually, CTS reaches 3500, Heat 6 threshold is satisfied.

- If you're using [Extra Options](https://github.com/ExOptsTeam), it's suggested to replicate the heat levels bonds from the mod's settings to avoid instability.

## Credits
- **Kevin4e** - Author of the mod.
- **ExOpts Team** - Mainly for discovering the instruction address to directly set a heat level, which simplified the development.

## Permissions
- You're **NOT** allowed to re-upload my mod anywhere else without my permission.
- You can use my mod on your modpack **as long as** you ask me privately.

## Troubleshooting
If you experience any unexpected behaviors, please submit an issue [here](https://github.com/Kevin4e/NFSHeatRaiseViaCTS/issues).
