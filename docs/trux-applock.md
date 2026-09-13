# TRUX App Lock

TRUX provides a native, hardware-backed App Lock feature using Android's `BiometricPrompt` framework.

---

## 1. Accessing App Lock

App Lock is integrated directly into the TRUX session drawer:
- Open the sidebar by swiping from the left edge (gesture-only navigation, no hamburger button).
- Tap **🔒 App Lock** near the bottom of the drawer (above Settings and footer).
- The sidebar indicates current status (**On** or **Off**).

---

## 2. Security & Credentials

- **Hardware Biometrics**: Supports Fingerprint and Face unlock where hardware and device enrollment are present.
- **Device Credential Fallback**: Fully supports device PIN, Pattern, and Password via `BiometricManager.Authenticators.DEVICE_CREDENTIAL`.
- **Zero Credential Storage**: TRUX never stores passwords, PINs, or biometric templates. Authentication is managed exclusively by Android OS Keyguard and Android Keystore.
- **Verification Gates**: Enabling App Lock, disabling App Lock, or changing timeout configuration requires successful biometric verification before settings are updated.

---

## 3. Lock Behavior & Privacy

- **Lock Screen**: A minimalist full-screen black overlay with the metallic TRUX emblem, status indicator, and `[ Unlock ]` action. No terminal content, session names, or command history are rendered while locked.
- **Recent Apps Privacy**: When App Lock is enabled, TRUX activates `WindowManager.LayoutParams.FLAG_SECURE` when locked or backgrounded, preventing terminal contents from appearing in Android's recent-apps overview or screenshots.
- **Background PTY Preservation**: When TRUX is locked, active PTY processes, build tasks, and background services (`MeridianService`) remain alive and uninterrupted.
- **Configurable Timeout**:
  - Immediately
  - After 30 seconds
  - After 1 minute
  - After 5 minutes
- **Configurable Triggers**:
  - Lock when app leaves foreground / backgrounded
  - Lock when screen turns off
