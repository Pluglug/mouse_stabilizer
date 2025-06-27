# Mouse Stabilizer - Coding Standards

## Overview
This document defines the coding standards and conventions for the Mouse Stabilizer project to ensure consistency, readability, and maintainability.

## File Organization

### Directory Structure
```
mouse_stabilizer/
├── include/                 # Header files
│   ├── core/               # Core functionality headers
│   ├── ui/                 # User interface headers
│   └── config/             # Configuration headers
├── src/ (future)           # Source files organized by module
├── docs/                   # Documentation
└── tests/ (future)         # Unit tests
```

### Header Files
- **One module per header**: Each header should focus on a single responsibility
- **Include guards**: Use `#ifndef MODULE_NAME_H` format
- **Forward declarations**: Minimize includes in headers using forward declarations
- **Module prefixes**: All functions should be prefixed with module name

## Naming Conventions

### Functions
- **Module prefix**: `ModuleName_FunctionName()`
- **PascalCase**: For module names and major function names
- **camelCase**: For internal/helper functions (optional)

**Examples:**
```c
// Core stabilizer functions
void StabilizerCore_Initialize(SmoothStabilizer* stabilizer);
void StabilizerCore_UpdatePosition(SmoothStabilizer* stabilizer);

// Mouse input functions  
bool MouseInput_RegisterRawInput(void);
void MouseInput_ProcessRawInput(LPARAM lParam);

// UI functions
bool TargetPointer_CreateWindow(void);
void TrayUI_UpdateIcon(void);
```

### Variables
- **Global variables**: `g_` prefix + PascalCase
- **Local variables**: snake_case or camelCase
- **Structure members**: snake_case
- **Constants/Macros**: UPPER_SNAKE_CASE

**Examples:**
```c
// Global variables
extern SmoothStabilizer g_stabilizer;
extern HWND g_hidden_window;

// Structure members
typedef struct {
    float follow_strength;
    bool dual_mode;
    DWORD delay_start_ms;
} SmoothStabilizer;

// Constants
#define UPDATE_INTERVAL_MS 8
#define DEFAULT_FOLLOW_STRENGTH 0.15f
```

### Types
- **Structures**: PascalCase
- **Enums**: PascalCase for type, UPPER_SNAKE_CASE for values
- **Typedefs**: PascalCase

**Examples:**
```c
typedef enum {
    EASE_LINEAR,
    EASE_IN,
    EASE_OUT,
    EASE_IN_OUT
} EaseType;

typedef struct {
    float x, y;
} MousePos;
```

## Code Style

### Indentation and Formatting
- **Indentation**: 4 spaces (no tabs)
- **Braces**: Allman style (opening brace on new line)
- **Line length**: Max 100 characters
- **Function spacing**: Blank line between functions

### Comments
- **Header comments**: Brief description of module purpose
- **Function comments**: Document parameters and return values for public APIs
- **Inline comments**: Explain complex logic, not obvious code
- **TODO comments**: Use `// TODO: description` format

**Example:**
```c
/**
 * Apply easing function to interpolation factor
 * @param t Interpolation factor (0.0 to 1.0)
 * @param ease_type Type of easing to apply
 * @return Eased interpolation factor
 */
float StabilizerCore_ApplyEasing(float t, EaseType ease_type);
```

### Error Handling
- **Return codes**: Use bool for success/failure, specific types for data
- **Null checks**: Always validate pointer parameters
- **Resource cleanup**: Always cleanup resources in error paths
- **Error propagation**: Propagate errors to callers with appropriate logging

### Logging Guidelines
- **Log Levels**: Use appropriate levels for different types of messages:
  - `LOG_ERROR`: Critical errors that prevent normal operation
  - `LOG_WARN`: Non-critical issues that should be noted
  - `LOG_INFO`: General operational information (default level)
  - `LOG_DEBUG`: Detailed debugging information for development
  - `LOG_TRACE`: Very detailed trace information for complex debugging

- **Log Content**: Include relevant context information:
  ```c
  LOG_ERROR("Failed to register raw input device: error code %lu", GetLastError());
  LOG_WARN("Target window creation failed, continuing without visual feedback");
  LOG_INFO("Stabilizer enabled with follow strength %.2f", g_stabilizer.follow_strength);
  LOG_DEBUG("Processing mouse delta: dx=%.1f, dy=%.1f", dx, dy);
  LOG_TRACE("Entering function %s with parameter %d", __func__, param);
  ```

- **Performance**: Logging is filtered by level, but avoid expensive operations in log statements
- **Privacy**: Never log sensitive user data or system information
- **Format**: Use clear, actionable messages that help with debugging

### Memory Management
- **Initialization**: Always initialize structures to zero: `= {0}`
- **Cleanup**: Ensure proper resource cleanup in exit paths
- **No dynamic allocation**: Prefer static allocation for this embedded-style application

## Module Responsibilities

### Core Modules
- **stabilizer_core**: Mathematical calculations, state management
- **mouse_input**: Raw input processing, Windows message handling

### UI Modules  
- **target_pointer**: Visual feedback overlay window
- **tray_ui**: System tray icon and context menu

### Config Modules
- **settings**: Configuration persistence, logging utilities

## API Design Principles

### Public APIs
- **Consistent naming**: All public functions use Module_Function pattern
- **Clear ownership**: Specify who owns/manages resources
- **Minimal coupling**: Reduce dependencies between modules
- **Error propagation**: Clear error handling strategy

### Internal Functions
- **Static functions**: Use static for module-internal functions
- **Helper functions**: Keep them focused and single-purpose
- **Parameter validation**: Validate inputs in public APIs only

## Documentation Requirements

### Code Documentation
- **Public headers**: Document all public APIs
- **Complex algorithms**: Explain the math/logic
- **Configuration**: Document all settings and their effects

### External Documentation
- **README.md**: User-facing documentation
- **CHANGELOG.md**: Track version changes
- **API.md**: Developer API reference

## Version Control

### Commit Messages
- **Format**: `Type: Brief description` followed by detailed explanation
- **Types**: feat, fix, refactor, docs, test, style
- **References**: Link to issues when applicable

### Branch Strategy
- **main**: Stable releases
- **develop**: Integration branch
- **feature/**: New feature development
- **refactor/**: Code improvement
- **fix/**: Bug fixes

## Example Code Structure

```c
// include/core/example_module.h
#ifndef EXAMPLE_MODULE_H
#define EXAMPLE_MODULE_H

#include <windows.h>
#include <stdbool.h>

// Constants
#define EXAMPLE_MAX_SIZE 256

// Types
typedef struct {
    int value;
    bool is_valid;
} ExampleData;

// Public API
bool ExampleModule_Initialize(void);
void ExampleModule_Cleanup(void);
bool ExampleModule_ProcessData(ExampleData* data);

#endif // EXAMPLE_MODULE_H
```

This standard ensures consistent, maintainable, and extensible code across the entire project.