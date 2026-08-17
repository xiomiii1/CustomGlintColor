# Custom Glint Color

Standalone LeviLaunchroid native mod that changes the Minecraft Bedrock enchantment glint color.

## Features
- Editable `#RRGGBB` glint color.
- Editable glint opacity from `0.0` to `1.0`.
- Persistent JSON configuration.
- LeviLaunchroid module-menu configuration.
- Uses only the four glint-rendering signatures required by this mod.

## Configuration
```json
{
  "Modules": {
    "Glint Color": {
      "masterEnabled": true,
      "glintColor": "#55FFFF",
      "glintOpacity": 0.8
    }
  }
}
```

The project contains only the mod-specific source and minimal native support code required for these hooks.
