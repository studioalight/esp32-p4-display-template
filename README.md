# ESP32-P4 Waveshare Touch LCD Template

**A working foundation for iteration.**

Template project for the [Waveshare ESP32-P4 WIFI6 Touch LCD 4B](https://www.waveshare.com/esp32-p4-wifi6-touch-lcd-4b.htm) development board — a 720×720 MIPI DSI display with capacitive touch, designed for rapid prototyping and provotypes.

---

## Hardware

| Feature | Specification |
|---------|--------------|
| SoC | ESP32-P4 (dual-core RISC-V, 400MHz) |
| Display | 4-inch IPS LCD, 720×720, MIPI DSI |
| Touch | GT911 capacitive touch controller |
| Flash | 32MB (QIO mode @ 80MHz) |
| PSRAM | 32MB external PSRAM |
| Audio | ES8311 codec + ES7210 ADC (optional) |

---

## What This Template Provides

- **Working display initialization** via BSP (Board Support Package)
- **LVGL graphics library** integration
- **Touch input** support (GT911)
- **Partition table** configured for 32MB flash
- **Serial output** via USB (for debugging)

The template compiles and runs. The display shows text. This is the solid foundation.

---

## Quick Start

### Prerequisites

- [ESP-IDF 5.4](https://docs.espressif.com/projects/esp-idf/en/v5.4/esp32p4/get-started/index.html) (stable, not dev)
- Python 3
- USB-C connection to the P4 board

### Build

```bash
# Set target (only needed once)
idf.py set-target esp32p4

# Build
idf.py build
```

### Flash

```bash
# Flash all components
idf.py flash

# Or flash specific addresses:
esptool.py --chip esp32p4 write_flash \
    0x2000 build/bootloader/bootloader.bin \
    0x8000 build/partition_table/partition-table.bin \
    0x10000 build/HelloWorld.bin
```

### Monitor

```bash
idf.py monitor
```

You should see:
```
=== HELLO P4 DISPLAY ===
D'ENT + Fredrik
I (342) HELLO_DISPLAY: Initializing...
I (352) HELLO_DISPLAY: Display ready 720x720
I (342) HELLO_DISPLAY: Done!
=== DISPLAY RUNNING ===
```

---

## The Marshmallow Approach

This template embodies **incremental iteration**:

1. **Start with what works** — display shows text
2. **Build on solid foundation** — add features layer by layer
3. **Iterate incrementally** — flash, test, adapt, repeat
4. **Keep what works** — discard what doesn't
5. **Bridge across gaps** — container builds, bridge flashes, hardware runs

> *"The marshmallow stands on solid foundation."*

---

## Project Structure

```
├── CMakeLists.txt          # Project configuration
├── partitions.csv          # Flash partition layout
├── sdkconfig.defaults      # Default SDK options
├── sdkconfig              # Active configuration (generated)
├── components/
│   └── bsp_extra/         # Board Support Package extensions
├── main/
│   ├── main.c             # Application entry point
│   └── CMakeLists.txt     # Component build rules
└── managed_components/    # ESP-IDF managed dependencies
    ├── lvgl__lvgl/        # LVGL graphics library
    ├── waveshare__esp32_p4_wifi6_touch_lcd_4b/  # Waveshare BSP
    └── st7703/            # Display driver
```

---

## Memory Layout

| Address | Content | Size |
|---------|---------|------|
| 0x2000 | Bootloader | 24KB |
| 0x8000 | Partition Table | 3KB |
| 0x10000 | Application | ~620KB |

---

## Key Configuration

### sdkconfig

Critical options for P4:
- `CONFIG_ESP32P4_REV_MIN_1=y` — Chip revision v1.0
- `CONFIG_SPIRAM=y` — Enable external PSRAM
- LVGL configured for 720×720 double-buffered rendering

The provided `sdkconfig` is already tuned for this hardware.

---

## Troubleshooting

### Display stays black

- Check backlight: `bsp_display_backlight_on()` must be called
- Verify MIPI DSI cable connection
- Check serial output for initialization errors

### ESP_ERR_NO_MEM

If display init fails with memory error:
- PSRAM must be enabled (`CONFIG_SPIRAM=y`)
- Check PSRAM initialization in boot log

### Touch not responding

- GT911 I2C address: default `0x5D` or `0x14`
- Check I2C pull-ups (see serial warning about resistances)

---

## Extending the Template

This is your starting point. Add:

- LVGL widgets (buttons, sliders, images)
- WiFi/Network connectivity
- Audio playback (ES8311)
- Touch gestures and UI
- Sleep modes and power management

**Principle:** Build to encounter the idea. See what the material teaches.

---

## Credits

**Created by:** D'ENT (Designed Entity)  
**Date:** March 10, 2026  
**Studio:** [Studio Alight](https://studioalight.com)  
**Context:** The great transition — biological to computational, replication to design

**Hardware:** [Waveshare ESP32-P4-WIFI6-Touch-LCD-4B](https://www.waveshare.com/esp32-p4-wifi6-touch-lcd-4b.htm)  
**Framework:** [ESP-IDF](https://github.com/espressif/esp-idf)  
**Graphics:** [LVGL](https://lvgl.io)

---

## License

Template components follow their respective licenses (ESP-IDF: Apache 2.0, LVGL: MIT). Add your own license for derivative work.

---

*"Iterate incrementally. Build through the night."*
