# Mouse Stabilizer for macOS

Experimental macOS menu bar implementation.

## Build

```bash
make -C macos
open macos/build/MouseStabilizerMac.app
```

The app uses a global event tap, so macOS must grant Accessibility permission.
If the cursor does not stabilize, open:

System Settings > Privacy & Security > Accessibility

Then enable Mouse Stabilizer and restart the app.

## Notes

- Settings and named profiles are stored in `UserDefaults`.
- Each ease type remembers its own follow strength and delay.
- This is a native macOS implementation, not a Win32 port.
