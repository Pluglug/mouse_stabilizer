import AppKit
import ApplicationServices
import CoreGraphics

private enum EaseType: Int, CaseIterable, Codable {
    case linear = 0
    case easeIn = 1
    case easeOut = 2
    case easeInOut = 3

    var title: String {
        switch self {
        case .linear: return "Linear"
        case .easeIn: return "Ease In"
        case .easeOut: return "Ease Out"
        case .easeInOut: return "Ease In-Out"
        }
    }
}

private struct StabilizerSettings: Codable {
    var enabled = true
    var easeType = EaseType.easeOut.rawValue
    var followStrengthByEase = [0.15, 0.15, 0.15, 0.15]
    var delayStartMsByEase = [150.0, 150.0, 150.0, 150.0]
    var dualMode = true
    var showTarget = true
    var alwaysShowTarget = false
    var targetSize = 8.0
    var targetAlpha = 0.70

    var ease: EaseType {
        get { EaseType(rawValue: easeType) ?? .easeOut }
        set { easeType = newValue.rawValue }
    }

    var followStrength: Double {
        get { followStrengthByEase[safe: easeType] ?? 0.15 }
        set { followStrengthByEase[easeType] = min(max(newValue, 0.05), 1.0) }
    }

    var delayStartMs: Double {
        get { delayStartMsByEase[safe: easeType] ?? 150.0 }
        set { delayStartMsByEase[easeType] = min(max(newValue, 0.0), 500.0) }
    }

    mutating func switchEase(_ next: EaseType) {
        ease = next
    }

    mutating func normalize() {
        if followStrengthByEase.count != 4 {
            followStrengthByEase = Array(repeating: followStrengthByEase.first ?? 0.15, count: 4)
        }
        if delayStartMsByEase.count != 4 {
            delayStartMsByEase = Array(repeating: delayStartMsByEase.first ?? 150.0, count: 4)
        }
        for i in 0..<4 {
            followStrengthByEase[i] = min(max(followStrengthByEase[i], 0.05), 1.0)
            delayStartMsByEase[i] = min(max(delayStartMsByEase[i], 0.0), 500.0)
        }
        if EaseType(rawValue: easeType) == nil {
            easeType = EaseType.easeOut.rawValue
        }
        targetSize = min(max(targetSize, 3.0), 24.0)
        targetAlpha = min(max(targetAlpha, 0.1), 1.0)
    }
}

private struct StabilizerProfile: Codable {
    var name: String
    var settings: StabilizerSettings
}

private extension Array {
    subscript(safe index: Int) -> Element? {
        indices.contains(index) ? self[index] : nil
    }
}

private final class SettingsStore {
    private let defaults = UserDefaults.standard
    private let settingsKey = "settings"
    private let profilesKey = "profiles"
    private let currentProfileKey = "currentProfile"

    var settings: StabilizerSettings {
        didSet {
            settings.normalize()
            saveSettings()
        }
    }

    var profiles: [StabilizerProfile] {
        didSet { saveProfiles() }
    }

    var currentProfileName: String {
        didSet { defaults.set(currentProfileName, forKey: currentProfileKey) }
    }

    init() {
        if let data = defaults.data(forKey: settingsKey),
           var decoded = try? JSONDecoder().decode(StabilizerSettings.self, from: data) {
            decoded.normalize()
            settings = decoded
        } else {
            settings = StabilizerSettings()
        }

        if let data = defaults.data(forKey: profilesKey),
           let decoded = try? JSONDecoder().decode([StabilizerProfile].self, from: data) {
            profiles = decoded
        } else {
            profiles = []
        }

        currentProfileName = defaults.string(forKey: currentProfileKey) ?? ""
    }

    func saveCurrentAsProfile(named rawName: String) -> Bool {
        let name = rawName.trimmingCharacters(in: .whitespacesAndNewlines)
        guard !name.isEmpty else { return false }

        if let index = profiles.firstIndex(where: { $0.name.caseInsensitiveCompare(name) == .orderedSame }) {
            profiles[index] = StabilizerProfile(name: name, settings: settings)
        } else {
            profiles.append(StabilizerProfile(name: name, settings: settings))
        }
        currentProfileName = name
        return true
    }

    func updateCurrentProfile() -> Bool {
        guard let index = profiles.firstIndex(where: { $0.name == currentProfileName }) else { return false }
        profiles[index].settings = settings
        return true
    }

    func renameCurrentProfile(to rawName: String) -> Bool {
        let name = rawName.trimmingCharacters(in: .whitespacesAndNewlines)
        guard !name.isEmpty,
              let index = profiles.firstIndex(where: { $0.name == currentProfileName }),
              !profiles.contains(where: { $0.name.caseInsensitiveCompare(name) == .orderedSame && $0.name != currentProfileName })
        else { return false }

        profiles[index].name = name
        currentProfileName = name
        return true
    }

    func deleteCurrentProfile() -> Bool {
        guard let index = profiles.firstIndex(where: { $0.name == currentProfileName }) else { return false }
        profiles.remove(at: index)
        currentProfileName = ""
        return true
    }

    func applyProfile(named name: String) -> Bool {
        guard let profile = profiles.first(where: { $0.name == name }) else { return false }
        var next = profile.settings
        next.normalize()
        settings = next
        currentProfileName = profile.name
        return true
    }

    private func saveSettings() {
        if let data = try? JSONEncoder().encode(settings) {
            defaults.set(data, forKey: settingsKey)
        }
    }

    private func saveProfiles() {
        if let data = try? JSONEncoder().encode(profiles) {
            defaults.set(data, forKey: profilesKey)
        }
    }
}

private final class TargetView: NSView {
    var size: CGFloat = 8.0
    var alpha: CGFloat = 0.7

    override var isFlipped: Bool { true }

    override func draw(_ dirtyRect: NSRect) {
        NSColor.clear.setFill()
        dirtyRect.fill()

        let center = CGPoint(x: bounds.midX, y: bounds.midY)
        NSColor(calibratedRed: 1.0, green: 0.25, blue: 0.25, alpha: alpha).setStroke()
        let path = NSBezierPath(ovalIn: CGRect(x: center.x - size, y: center.y - size, width: size * 2, height: size * 2))
        path.lineWidth = 2
        path.stroke()
    }
}

private final class TargetOverlay {
    private let window: NSPanel
    private let view = TargetView()

    init() {
        window = NSPanel(
            contentRect: NSRect(x: 0, y: 0, width: 48, height: 48),
            styleMask: [.borderless, .nonactivatingPanel],
            backing: .buffered,
            defer: false
        )
        window.backgroundColor = .clear
        window.isOpaque = false
        window.hasShadow = false
        window.ignoresMouseEvents = true
        window.level = .screenSaver
        window.collectionBehavior = [.canJoinAllSpaces, .fullScreenAuxiliary, .stationary]
        window.contentView = view
    }

    func update(at quartzPoint: CGPoint, settings: StabilizerSettings, current: CGPoint, target: CGPoint) {
        guard settings.enabled, settings.showTarget else {
            window.orderOut(nil)
            return
        }

        let distance = hypot(target.x - current.x, target.y - current.y)
        guard settings.alwaysShowTarget || distance >= 5.0 else {
            window.orderOut(nil)
            return
        }

        let size = CGFloat(settings.targetSize)
        let side = max(size * 4, 32)
        view.size = size
        view.alpha = CGFloat(settings.targetAlpha)
        view.needsDisplay = true

        let cocoa = Self.cocoaPoint(fromQuartz: quartzPoint)
        window.setFrame(NSRect(x: cocoa.x - side / 2, y: cocoa.y - side / 2, width: side, height: side), display: true)
        window.orderFrontRegardless()
    }

    private static func cocoaPoint(fromQuartz point: CGPoint) -> CGPoint {
        let maxY = NSScreen.screens.map { $0.frame.maxY }.max() ?? NSScreen.main?.frame.height ?? 0
        return CGPoint(x: point.x, y: maxY - point.y)
    }
}

private final class StabilizerEngine {
    var settings: StabilizerSettings {
        didSet {
            settings.normalize()
            if !settings.enabled {
                overlay.update(at: target, settings: settings, current: current, target: target)
            }
        }
    }

    private var eventTap: CFMachPort?
    private var runLoopSource: CFRunLoopSource?
    private var timer: DispatchSourceTimer?
    private var target: CGPoint
    private var current: CGPoint
    private var velocity = 0.0
    private var lastTargetUpdate = CFAbsoluteTimeGetCurrent()
    private var movementStart: CFAbsoluteTime?
    private let overlay = TargetOverlay()

    init(settings: StabilizerSettings) {
        self.settings = settings
        let location = CGEvent(source: nil)?.location ?? CGPoint(x: 100, y: 100)
        self.target = location
        self.current = location
    }

    func start() -> Bool {
        startTimer()
        return startEventTap()
    }

    func stop() {
        if let eventTap {
            CGEvent.tapEnable(tap: eventTap, enable: false)
        }
        if let runLoopSource {
            CFRunLoopRemoveSource(CFRunLoopGetCurrent(), runLoopSource, .commonModes)
        }
        timer?.cancel()
        timer = nil
        self.eventTap = nil
        self.runLoopSource = nil
    }

    private func startTimer() {
        let timer = DispatchSource.makeTimerSource(queue: .main)
        timer.schedule(deadline: .now(), repeating: .milliseconds(8), leeway: .milliseconds(1))
        timer.setEventHandler { [weak self] in
            self?.updateCursor()
        }
        timer.resume()
        self.timer = timer
    }

    private func startEventTap() -> Bool {
        let mask =
            (1 << CGEventType.mouseMoved.rawValue) |
            (1 << CGEventType.leftMouseDragged.rawValue) |
            (1 << CGEventType.rightMouseDragged.rawValue) |
            (1 << CGEventType.otherMouseDragged.rawValue)

        let refcon = Unmanaged.passUnretained(self).toOpaque()
        guard let tap = CGEvent.tapCreate(
            tap: .cgSessionEventTap,
            place: .headInsertEventTap,
            options: .defaultTap,
            eventsOfInterest: CGEventMask(mask),
            callback: eventTapCallback,
            userInfo: refcon
        ) else {
            return false
        }

        eventTap = tap
        runLoopSource = CFMachPortCreateRunLoopSource(kCFAllocatorDefault, tap, 0)
        CFRunLoopAddSource(CFRunLoopGetCurrent(), runLoopSource, .commonModes)
        CGEvent.tapEnable(tap: tap, enable: true)
        return true
    }

    fileprivate func handle(proxy: CGEventTapProxy, type: CGEventType, event: CGEvent) -> Unmanaged<CGEvent>? {
        if type == .tapDisabledByTimeout || type == .tapDisabledByUserInput {
            if let eventTap {
                CGEvent.tapEnable(tap: eventTap, enable: true)
            }
            return Unmanaged.passUnretained(event)
        }

        guard settings.enabled else {
            target = event.location
            current = event.location
            return Unmanaged.passUnretained(event)
        }

        let dx = Double(event.getIntegerValueField(.mouseEventDeltaX))
        let dy = Double(event.getIntegerValueField(.mouseEventDeltaY))
        guard dx != 0 || dy != 0 else {
            return nil
        }

        let oldTarget = target
        target.x += dx
        target.y += dy
        target = clampToDisplays(target)

        let now = CFAbsoluteTimeGetCurrent()
        let dt = max(now - lastTargetUpdate, 0.001)
        velocity = hypot(target.x - oldTarget.x, target.y - oldTarget.y) / dt * 0.7 + velocity * 0.3
        lastTargetUpdate = now
        return nil
    }

    private func updateCursor() {
        guard settings.enabled else { return }

        let distance = hypot(target.x - current.x, target.y - current.y)
        if distance < 0.5 {
            movementStart = nil
            overlay.update(at: target, settings: settings, current: current, target: target)
            return
        }

        let now = CFAbsoluteTimeGetCurrent()
        if movementStart == nil {
            movementStart = now
        }

        if (now - (movementStart ?? now)) * 1000.0 < settings.delayStartMs {
            overlay.update(at: target, settings: settings, current: current, target: target)
            return
        }

        var follow = settings.followStrength
        if settings.dualMode && velocity > 100.0 {
            follow = min(follow * 3.0, 0.8)
        }

        let factor = eased(follow, settings.ease)
        current.x += (target.x - current.x) * factor
        current.y += (target.y - current.y) * factor
        current = clampToDisplays(current)

        CGWarpMouseCursorPosition(current)
        overlay.update(at: target, settings: settings, current: current, target: target)
    }

    private func eased(_ t: Double, _ ease: EaseType) -> Double {
        let t = min(max(t, 0.0), 1.0)
        switch ease {
        case .linear:
            return t
        case .easeIn:
            return t * t
        case .easeOut:
            return 1.0 - (1.0 - t) * (1.0 - t)
        case .easeInOut:
            if t < 0.5 {
                return 2.0 * t * t
            }
            return 1.0 - 2.0 * (1.0 - t) * (1.0 - t)
        }
    }

    private func clampToDisplays(_ point: CGPoint) -> CGPoint {
        let displays = NSScreen.screens.map { $0.deviceDescription[NSDeviceDescriptionKey("NSScreenNumber")] as? CGDirectDisplayID }
            .compactMap { $0 }
            .map { CGDisplayBounds($0) }
        let union = displays.reduce(CGRect.null) { $0.union($1) }
        guard !union.isNull else { return point }

        return CGPoint(
            x: min(max(point.x, union.minX), union.maxX - 1),
            y: min(max(point.y, union.minY), union.maxY - 1)
        )
    }
}

private let eventTapCallback: CGEventTapCallBack = { proxy, type, event, refcon in
    guard let refcon else {
        return Unmanaged.passUnretained(event)
    }
    let engine = Unmanaged<StabilizerEngine>.fromOpaque(refcon).takeUnretainedValue()
    return engine.handle(proxy: proxy, type: type, event: event)
}

private final class SettingsWindowController: NSWindowController {
    private let store: SettingsStore
    private let engine: StabilizerEngine
    private let onChange: () -> Void

    private let profilePopup = NSPopUpButton()
    private let profileNameField = NSTextField()
    private let enabledCheck = NSButton(checkboxWithTitle: "Enable Mouse Stabilizer", target: nil, action: nil)
    private let easePopup = NSPopUpButton()
    private let followSlider = NSSlider(value: 0.15, minValue: 0.05, maxValue: 1.0, target: nil, action: nil)
    private let followValue = NSTextField(labelWithString: "")
    private let delaySlider = NSSlider(value: 150, minValue: 0, maxValue: 500, target: nil, action: nil)
    private let delayValue = NSTextField(labelWithString: "")
    private let dualModeCheck = NSButton(checkboxWithTitle: "Dual Mode", target: nil, action: nil)
    private let showTargetCheck = NSButton(checkboxWithTitle: "Show target pointer", target: nil, action: nil)
    private let alwaysTargetCheck = NSButton(checkboxWithTitle: "Always show target", target: nil, action: nil)
    private let targetSizeSlider = NSSlider(value: 8, minValue: 3, maxValue: 24, target: nil, action: nil)
    private let targetSizeValue = NSTextField(labelWithString: "")
    private var updating = false

    init(store: SettingsStore, engine: StabilizerEngine, onChange: @escaping () -> Void) {
        self.store = store
        self.engine = engine
        self.onChange = onChange

        let window = NSWindow(
            contentRect: NSRect(x: 0, y: 0, width: 460, height: 440),
            styleMask: [.titled, .closable, .miniaturizable],
            backing: .buffered,
            defer: false
        )
        window.title = "Mouse Stabilizer Settings"
        super.init(window: window)
        buildUI()
        refresh()
    }

    required init?(coder: NSCoder) {
        fatalError("init(coder:) has not been implemented")
    }

    func refresh() {
        updating = true
        profilePopup.removeAllItems()
        profilePopup.addItems(withTitles: store.profiles.map(\.name))
        if !store.currentProfileName.isEmpty {
            profilePopup.selectItem(withTitle: store.currentProfileName)
            profileNameField.stringValue = store.currentProfileName
        } else {
            profileNameField.stringValue = ""
        }

        let settings = store.settings
        enabledCheck.state = settings.enabled ? .on : .off
        easePopup.selectItem(at: settings.ease.rawValue)
        followSlider.doubleValue = settings.followStrength
        delaySlider.doubleValue = settings.delayStartMs
        dualModeCheck.state = settings.dualMode ? .on : .off
        showTargetCheck.state = settings.showTarget ? .on : .off
        alwaysTargetCheck.state = settings.alwaysShowTarget ? .on : .off
        targetSizeSlider.doubleValue = settings.targetSize
        updateValueLabels()
        updating = false
    }

    private func buildUI() {
        guard let contentView = window?.contentView else { return }

        let stack = NSStackView()
        stack.orientation = .vertical
        stack.alignment = .leading
        stack.spacing = 12
        stack.translatesAutoresizingMaskIntoConstraints = false
        contentView.addSubview(stack)

        NSLayoutConstraint.activate([
            stack.leadingAnchor.constraint(equalTo: contentView.leadingAnchor, constant: 18),
            stack.trailingAnchor.constraint(equalTo: contentView.trailingAnchor, constant: -18),
            stack.topAnchor.constraint(equalTo: contentView.topAnchor, constant: 18)
        ])

        profilePopup.target = self
        profilePopup.action = #selector(loadSelectedProfile)
        stack.addArrangedSubview(row("Profile", profilePopup, button("Load", #selector(loadSelectedProfile))))
        stack.addArrangedSubview(row("Profile Name", profileNameField, button("Save As", #selector(saveAsProfile))))
        stack.addArrangedSubview(buttonRow([
            button("Update Profile", #selector(updateProfile)),
            button("Rename", #selector(renameProfile)),
            button("Delete", #selector(deleteProfile))
        ]))

        enabledCheck.target = self
        enabledCheck.action = #selector(controlChanged)
        stack.addArrangedSubview(enabledCheck)

        easePopup.addItems(withTitles: EaseType.allCases.map(\.title))
        easePopup.target = self
        easePopup.action = #selector(easeChanged)
        stack.addArrangedSubview(row("Ease Type", easePopup))

        followSlider.target = self
        followSlider.action = #selector(controlChanged)
        stack.addArrangedSubview(sliderRow("Follow Strength", followSlider, followValue))

        delaySlider.target = self
        delaySlider.action = #selector(controlChanged)
        stack.addArrangedSubview(sliderRow("Delay Start", delaySlider, delayValue))

        dualModeCheck.target = self
        dualModeCheck.action = #selector(controlChanged)
        showTargetCheck.target = self
        showTargetCheck.action = #selector(controlChanged)
        alwaysTargetCheck.target = self
        alwaysTargetCheck.action = #selector(controlChanged)
        stack.addArrangedSubview(dualModeCheck)
        stack.addArrangedSubview(showTargetCheck)
        stack.addArrangedSubview(alwaysTargetCheck)

        targetSizeSlider.target = self
        targetSizeSlider.action = #selector(controlChanged)
        stack.addArrangedSubview(sliderRow("Target Size", targetSizeSlider, targetSizeValue))
    }

    private func row(_ label: String, _ views: NSView...) -> NSStackView {
        let stack = NSStackView()
        stack.orientation = .horizontal
        stack.alignment = .centerY
        stack.spacing = 8
        let labelView = NSTextField(labelWithString: label)
        labelView.widthAnchor.constraint(equalToConstant: 110).isActive = true
        stack.addArrangedSubview(labelView)
        for view in views {
            view.widthAnchor.constraint(greaterThanOrEqualToConstant: view is NSButton ? 70 : 190).isActive = true
            stack.addArrangedSubview(view)
        }
        return stack
    }

    private func buttonRow(_ buttons: [NSButton]) -> NSStackView {
        let stack = NSStackView()
        stack.orientation = .horizontal
        stack.alignment = .centerY
        stack.spacing = 8
        let spacer = NSView()
        spacer.widthAnchor.constraint(equalToConstant: 110).isActive = true
        stack.addArrangedSubview(spacer)
        for button in buttons {
            stack.addArrangedSubview(button)
        }
        return stack
    }

    private func sliderRow(_ label: String, _ slider: NSSlider, _ value: NSTextField) -> NSStackView {
        slider.widthAnchor.constraint(equalToConstant: 220).isActive = true
        value.widthAnchor.constraint(equalToConstant: 70).isActive = true
        return row(label, slider, value)
    }

    private func button(_ title: String, _ action: Selector) -> NSButton {
        let button = NSButton(title: title, target: self, action: action)
        button.bezelStyle = .rounded
        return button
    }

    @objc private func controlChanged() {
        guard !updating else { return }
        var settings = store.settings
        settings.enabled = enabledCheck.state == .on
        settings.followStrength = followSlider.doubleValue
        settings.delayStartMs = delaySlider.doubleValue
        settings.dualMode = dualModeCheck.state == .on
        settings.showTarget = showTargetCheck.state == .on
        settings.alwaysShowTarget = alwaysTargetCheck.state == .on
        settings.targetSize = targetSizeSlider.doubleValue
        store.settings = settings
        engine.settings = store.settings
        updateValueLabels()
        onChange()
    }

    @objc private func easeChanged() {
        guard !updating, let next = EaseType(rawValue: easePopup.indexOfSelectedItem) else { return }
        var settings = store.settings
        settings.followStrength = followSlider.doubleValue
        settings.delayStartMs = delaySlider.doubleValue
        settings.switchEase(next)
        store.settings = settings
        engine.settings = store.settings
        refresh()
        onChange()
    }

    @objc private func loadSelectedProfile() {
        guard let title = profilePopup.selectedItem?.title, store.applyProfile(named: title) else { return }
        engine.settings = store.settings
        refresh()
        onChange()
    }

    @objc private func saveAsProfile() {
        controlChanged()
        _ = store.saveCurrentAsProfile(named: profileNameField.stringValue)
        refresh()
        onChange()
    }

    @objc private func updateProfile() {
        controlChanged()
        _ = store.updateCurrentProfile()
        refresh()
        onChange()
    }

    @objc private func renameProfile() {
        _ = store.renameCurrentProfile(to: profileNameField.stringValue)
        refresh()
        onChange()
    }

    @objc private func deleteProfile() {
        _ = store.deleteCurrentProfile()
        refresh()
        onChange()
    }

    private func updateValueLabels() {
        followValue.stringValue = String(format: "%.2f", followSlider.doubleValue)
        delayValue.stringValue = String(format: "%.0f ms", delaySlider.doubleValue)
        targetSizeValue.stringValue = String(format: "%.0f px", targetSizeSlider.doubleValue)
    }
}

@main
private final class AppDelegate: NSObject, NSApplicationDelegate {
    private let store = SettingsStore()
    private var engine: StabilizerEngine!
    private var statusItem: NSStatusItem!
    private var settingsWindow: SettingsWindowController!

    func applicationDidFinishLaunching(_ notification: Notification) {
        NSApp.setActivationPolicy(.accessory)
        requestAccessibility()

        engine = StabilizerEngine(settings: store.settings)
        settingsWindow = SettingsWindowController(store: store, engine: engine) { [weak self] in
            self?.rebuildMenu()
        }

        statusItem = NSStatusBar.system.statusItem(withLength: NSStatusItem.variableLength)
        statusItem.button?.title = "MS"
        rebuildMenu()

        if !engine.start() {
            statusItem.button?.title = "MS!"
            showAccessibilityAlert()
        }
    }

    private func rebuildMenu() {
        let menu = NSMenu()
        let toggle = NSMenuItem(title: store.settings.enabled ? "Disable Stabilizer" : "Enable Stabilizer", action: #selector(toggleEnabled), keyEquivalent: "")
        toggle.target = self
        menu.addItem(toggle)

        let profiles = NSMenuItem(title: "Profiles", action: nil, keyEquivalent: "")
        let submenu = NSMenu()
        if store.profiles.isEmpty {
            submenu.addItem(NSMenuItem(title: "(no saved profiles)", action: nil, keyEquivalent: ""))
        } else {
            for profile in store.profiles {
                let item = NSMenuItem(title: profile.name, action: #selector(applyProfile(_:)), keyEquivalent: "")
                item.target = self
                item.state = profile.name == store.currentProfileName ? .on : .off
                submenu.addItem(item)
            }
        }
        profiles.submenu = submenu
        menu.addItem(profiles)

        menu.addItem(.separator())
        let settings = NSMenuItem(title: "Settings...", action: #selector(showSettings), keyEquivalent: ",")
        settings.target = self
        menu.addItem(settings)

        let permission = NSMenuItem(title: "Open Accessibility Settings", action: #selector(openAccessibilitySettings), keyEquivalent: "")
        permission.target = self
        menu.addItem(permission)

        menu.addItem(.separator())
        let quit = NSMenuItem(title: "Quit", action: #selector(quit), keyEquivalent: "q")
        quit.target = self
        menu.addItem(quit)
        statusItem.menu = menu
    }

    private func requestAccessibility() {
        let options = [kAXTrustedCheckOptionPrompt.takeUnretainedValue() as String: true] as CFDictionary
        _ = AXIsProcessTrustedWithOptions(options)
    }

    private func showAccessibilityAlert() {
        let alert = NSAlert()
        alert.messageText = "Accessibility permission is required"
        alert.informativeText = "Enable Mouse Stabilizer in System Settings > Privacy & Security > Accessibility, then restart the app."
        alert.addButton(withTitle: "Open Settings")
        alert.addButton(withTitle: "OK")
        if alert.runModal() == .alertFirstButtonReturn {
            openAccessibilitySettings()
        }
    }

    @objc private func toggleEnabled() {
        var settings = store.settings
        settings.enabled.toggle()
        store.settings = settings
        engine.settings = settings
        settingsWindow.refresh()
        rebuildMenu()
    }

    @objc private func applyProfile(_ sender: NSMenuItem) {
        guard store.applyProfile(named: sender.title) else { return }
        engine.settings = store.settings
        settingsWindow.refresh()
        rebuildMenu()
    }

    @objc private func showSettings() {
        settingsWindow.showWindow(nil)
        settingsWindow.window?.center()
        NSApp.activate(ignoringOtherApps: true)
    }

    @objc private func openAccessibilitySettings() {
        let url = URL(string: "x-apple.systempreferences:com.apple.preference.security?Privacy_Accessibility")!
        NSWorkspace.shared.open(url)
    }

    @objc private func quit() {
        engine.stop()
        NSApp.terminate(nil)
    }
}
