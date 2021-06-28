# VelocityVectorsPlugin
Rocket League BakkesMod plugin creates velocity vectors for the car and ball.

Add your path to BakkesModSDK in environment variables as "BakkesModSDK"

Post build command copys .dll file to BakkesModSDK/plugins folder with "bakkes_patchplugin.py" script.

"bakkes_patchplugin.py" script needs one line modification, change example string to your BakkesMod plugin folder.

Once built, load in BakkesMod command terminal with

    "plugin load VelocityVectorsPlugin"
    "cl_show_vectors 1"

![vectors](https://user-images.githubusercontent.com/37971619/123708127-e5329e00-d838-11eb-94bb-71736140696e.png)

Screenshots above shows what vecotrs will look like. Useful for practicing air-dribbling


Possible Problems:
- BakkesModSDK environment variable not set correctly, affects paths to include and lib directories.
- Post-build command: python not installed, python not a path variable, "bakkes_patchplugin.py" not pointing to proper plugins directory.
- Rocket League screen refresh rate set above 60 FPS, change refresh rate in Rocket League Settings.
