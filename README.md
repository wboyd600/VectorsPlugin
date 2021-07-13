# VelocityVectorsPlugin
Rocket League BakkesMod plugin creates velocity vectors for the car and ball.

Add your path to BakkesModSDK in environment variables as "BakkesModSDK"

Post build command copys .dll file to BakkesModSDK/plugins folder with "bakkes_patchplugin.py" script.

"bakkes_patchplugin.py" script needs one line modification, change example string to your BakkesMod plugin folder.

Once built, load in BakkesMod command terminal with

    "plugin load velocityvectorplugin"
    "cl_show_vectors 1"

Alternatively, one can control cl_show_vectors and cl_vector_color from BakkesMod settings

1. Copy `VelocityVectorPlugin.dll` to `<bakkesmod_directory>/plugins/`
2. Copy `velocityvectorplugin.set` to `<bakkesmod_directory>/plugins/settings/`
3. In the game console run `plugin load velocityvectorplugin`
4. To refresh settings run `cl_settings_refreshplugins`

![vectors](https://user-images.githubusercontent.com/37971619/123708127-e5329e00-d838-11eb-94bb-71736140696e.png)
![vectors_2](https://user-images.githubusercontent.com/37971619/123708409-5a9e6e80-d839-11eb-8756-eb86f64a9cc3.png)

Screenshots above shows what vectors will look like. Useful for practicing air-dribbling


Possible Problems:
- BakkesModSDK environment variable not set correctly, affects paths to include and lib directories.
- Post-build command: python not installed, python not a path variable, "bakkes_patchplugin.py" not pointing to proper plugins directory.
- Rocket League screen refresh rate set above 60 FPS, change refresh rate in Rocket League Settings.
