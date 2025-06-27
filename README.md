# Mouse Stabilizer

Windows application providing paint-style cursor smoothing for presentations and precise work. Reduces hand tremor effects while maintaining natural mouse control.

## Features

- **Paint-style Following**: Cursor smoothly follows target position like drawing software
- **Configurable Easing**: Linear, Ease In, Ease Out, Ease In-Out smoothing curves  
- **Visual Feedback**: Optional red target pointer shows actual mouse position
- **Delay Start**: Configurable delay before smoothing begins (0-500ms)
- **Dual Mode**: Adaptive smoothing that responds to movement velocity
- **Hotkey Toggle**: Ctrl+Alt+S to instantly enable/disable
- **System Tray Control**: Right-click for real-time setting adjustments
- **High Precision**: Raw Input API for low-latency processing

## Quick Start

1. **Compile**: `make` (requires MinGW or MSVC)
2. **Run**: `mouse_stabilizer.exe`  
3. **Toggle**: Press `Ctrl+Alt+S` or right-click tray icon
4. **Configure**: Right-click tray icon for settings menu

## Build Methods

### MinGW-w64 (Recommended)
```bash
make
```

### Visual Studio
```bash
cl main.c mouse_input.c smooth_engine.c hotkey.c tray_ui.c target_pointer.c config.c /Fe:mouse_stabilizer.exe user32.lib kernel32.lib shell32.lib gdi32.lib
```

## Key Settings

- **Follow Strength** (0.05-1.0): How quickly cursor follows target
- **Ease Type**: Smoothing curve (Linear/Ease In/Out/In-Out)  
- **Delay Start** (0-500ms): Wait time before following starts
- **Dual Mode**: Enable faster following for quick movements
- **Target Distance** (2-20px): Threshold for showing target pointer

## Recommended Configurations

- **Presentations**: Follow 0.1, Ease In-Out, Delay 150ms
- **Drawing/Design**: Follow 0.15, Ease Out, Delay 100ms  
- **General Use**: Follow 0.2, Linear, Delay 50ms

## Technical Specifications

- **Compatibility**: Windows 7+ (x86/x64)
- **CPU Usage**: < 1%
- **Memory Usage**: < 5MB  
- **Latency**: < 8ms (configurable update interval)
- **Architecture**: Modular C codebase with separated concerns

## Generated Files

- `mouse_stabilizer.ini` - Settings (auto-created)
- `mouse_stabilizer.log` - Debug log (auto-created)

## Troubleshooting

- **Admin Rights Required**: Run as administrator if UAC is enabled
- **Hotkey Not Working**: Check for conflicts with other applications
- **No Smoothing Effect**: Increase Follow Strength or decrease Delay
- **Target Pointer Not Visible**: Decrease Target Distance threshold

## Architecture

- **Core Engine**: `smooth_engine.c` - Mathematical smoothing algorithms
- **Input Processing**: `mouse_input.c` - Raw Input API handling
- **UI Components**: `tray_ui.c`, `target_pointer.c` - User interface
- **Configuration**: `config.c` - Settings persistence and logging
- **Main Loop**: `main.c` - Application initialization and message loop

## License

See source code for licensing information.