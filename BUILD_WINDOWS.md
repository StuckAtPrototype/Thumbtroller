# Thumbtroller Firmware: Windows Build Guide

This guide is for **Windows only**. It walks you through installing ESP-IDF 5.5 (via Git), setting up the build environment in **Command Prompt** (not PowerShell), cloning the Thumbtroller repo, and building and flashing the firmware.

**Requirements:** [Git for Windows](https://gitforwindows.org/) installed. A USB-to-Serial adapter for flashing (see [README](README.md)).  
If you don't have Git: download the installer from [gitforwindows.org](https://gitforwindows.org/), run it, and accept the defaults.

---

## 1. Install ESP-IDF 5.5 — Git clone + install.bat

Use **Git** to clone ESP-IDF 5.5, then run the Windows install script.

1. Open **Command Prompt** (Win+R → `cmd` → Enter).
2. Choose a directory **without spaces**, under 90 characters (e.g. `C:\esp` or `%userprofile%\esp`). Create it if needed:
   ```cmd
   mkdir C:\esp
   cd C:\esp
   ```
3. Clone the ESP-IDF repo on the **release/v5.5** branch:
   ```cmd
   git clone -b release/v5.5 --recursive https://github.com/espressif/esp-idf.git
   cd esp-idf
   ```
   - Repo: [https://github.com/espressif/esp-idf](https://github.com/espressif/esp-idf) (branch [release/v5.5](https://github.com/espressif/esp-idf/tree/release/v5.5)).
   - `--recursive` pulls submodules required by ESP-IDF.
4. Install tools (Python, cross-compilers, CMake, Ninja, etc.):
   ```cmd
   install.bat
   ```
   Run this **once** after the clone; it may take several minutes.
5. **Path rules:** No spaces, parentheses, or special characters in the path; total path length under 90 characters. See [Espressif Windows setup](https://docs.espressif.com/projects/esp-idf/en/v5.5.1/esp32h2/get-started/windows-setup.html) for details.

---

## 2. Export IDF in Command Prompt (every session)

**Use Command Prompt (cmd.exe), not PowerShell.** PowerShell is not supported for the export step (`export.ps1` can fail or behave differently). Use **Command Prompt** only.

**Every time** you open a new terminal to build or flash, you must run the export in that session:

1. Open **Command Prompt** (Win+R → `cmd` → Enter).
2. Go to your ESP-IDF directory, e.g.:
   ```cmd
   cd C:\esp\esp-idf
   ```
   (or `cd %userprofile%\esp\esp-idf` if you cloned there)
3. Run the batch export script:
   ```cmd
   export.bat
   ```

**Tip:** You can create a shortcut that starts in your esp-idf directory and runs `export.bat`, so each new Command Prompt is already set up; otherwise run `cd` + `export.bat` manually every time.

---

## 3. Get the Thumbtroller repository

**Option A – Git clone**

From a folder of your choice (e.g. `C:\esp` or `C:\dev`):

```cmd
git clone https://github.com/StuckAtPrototype/Thumbtroller.git
cd Thumbtroller
```

**Option B – Download ZIP from GitHub**

1. Open [https://github.com/StuckAtPrototype/Thumbtroller](https://github.com/StuckAtPrototype/Thumbtroller).
2. Click **Code** → **Download ZIP**.
3. Unzip to a path **without spaces** (e.g. `C:\dev\Thumbtroller`).
4. In Command Prompt:
   ```cmd
   cd C:\dev\Thumbtroller
   ```
   (adjust path to where you extracted the ZIP)

**Reminder:** After obtaining the repo (either method), run **Step 2** (open Command Prompt and run `export.bat` from the IDF directory) before building.

---

## 4. Build the firmware

All IDF commands must be run from the **firmware** directory inside the Thumbtroller repo.

```cmd
cd Thumbtroller\firmware
```

(or `cd firmware` if you are already in the repo root)

**Set target** (only needed once per clone/fresh copy, or after `idf.py fullclean`):

```cmd
idf.py set-target esp32h2
```

**Build:**

```cmd
idf.py build
```

**Optional:** To reconfigure (Wi-Fi, BLE, etc.):

```cmd
idf.py menuconfig
```

---

## 5. Flash the firmware

A **USB-to-Serial** adapter is required. Connect the Thumbtroller board and note the COM port (e.g. Device Manager → Ports (COM & LPT) → "USB Serial Port (COM3)").

**Flash** (replace `COM3` with your port):

```cmd
idf.py -p COM3 flash
```

**Flash and open serial monitor:**

```cmd
idf.py -p COM3 flash monitor
```

Exit monitor: **Ctrl+]**.

You can also run `idf.py -p COM3 flash` without running `idf.py build` first; it will build automatically if needed.

---

## Troubleshooting

- [Espressif Windows setup](https://docs.espressif.com/projects/esp-idf/en/v5.5.1/esp32h2/get-started/windows-setup.html)
- [Establish Serial Connection with ESP32-H2](https://docs.espressif.com/projects/esp-idf/en/v5.5.1/esp32h2/get-started/establish-serial-connection.html)
