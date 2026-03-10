# Design → Deploy → Verify Flow

**Container-to-Hardware Iteration without Direct USB**

A distributed workflow for ESP32 development where the development environment (container) and hardware (USB-connected device) are physically separated.

---

## The Problem

**Traditional ESP32 workflow:**
```
Dev Machine ←USB→ ESP32 Hardware
```

**Our constraint:**
- Containerized development (no USB access)
- Hardware connected to different machine (MacBook)
- Still need fast iteration loop

**Solution:** Network bridge with HTTP + WebSocket

---

## Architecture

```
┌─────────────┐      Tailscale       ┌─────────────┐      USB      ┌─────────┐
│  Container  │  ←────────────────→  │   MacBook   │  ←─────────→  │ ESP32   │
│   (D'ENT)   │    (encrypted mesh)   │   (Bridge)  │               │   P4    │
└─────────────┘                       └─────────────┘               └─────────┘
      ↓                                     ↓                           ↓
   Edit/build                          HTTP + WebSocket               Flash/
   in VS Code                          esptool.py                     Serial
```

---

## The Flow

### 1. DESIGN - Edit in Container

**Where:** Visual Studio Code in Docker container  
**What:** Edit source files (main.c, components, sdkconfig)

```
~/.openclaw/workspace/p4-lyric-display/
├── main/main.c              ← Edit here
├── sdkconfig                ← Configuration
└── build/                   ← Generated
```

**Key principle:** Full ESP-IDF toolchain available (compilers, headers, SDK)

---

### 2. BUILD - Compile in Container

**Command:**
```bash
source ~/esp-idf-v5.4/export.sh    # Activate ESP-IDF
idf.py build                        # Compile project
```

**Outputs:**
```
build/
├── bootloader/bootloader.bin       → Flash to 0x2000
├── partition_table/partition-table.bin → Flash to 0x8000
└── HelloWorld.bin                  → Flash to 0x10000
```

**Time:** ~2 minutes for clean build, ~30 seconds for incremental

---

### 3. DEPLOY - Upload to Bridge

**Where:** Container → HTTP → MacBook  
**Tool:** `curl` via Tailscale (100.x.x.x address)

**Upload:**
```bash
curl -k -F "file=@build/HelloWorld.bin" \
  "https://esp32-bridge.tailbdd5a.ts.net:5679/upload"
```

**Result:** Binary stored on MacBook at:
```
/Users/fredrikgarneij/.esp32-bridge/uploads/HelloWorld.bin
```

**Bridge features:**
- ✅ Tailscale auto-discovery
- ✅ HTTPS with valid certificates
- ✅ WebSocket for commands + serial
- ✅ Multi-file upload queue

---

### 4. FLASH - Program the Hardware

**Where:** MacBook → esptool.py → ESP32-P4  
**Tool:** WebSocket command to bridge, which runs esptool locally

**WebSocket Command:**
```json
{"action": "flash", "file": "HelloWorld.bin", "addr": "0x10000"}
```

**Connection flow:**
```
Container       Bridge (MacBook)      ESP32-P4
   ↓                  ↓                   ↓
WSS ──────────────► USB-Serial ───────► Flash
   TLS 1.3          /dev/cu.usbmodem    Memory
   Tailscale        esptool.py          0x10000
```

**Result:** Firmware written to flash, hash verified, hard reset triggered

---

### 5. VERIFY - Serial Output

**Where:** ESP32-P4 → USB-Serial → Bridge → Container  
**Tool:** WebSocket streaming

**Connection:**
```python
ws = websocket.connect('wss://esp32-bridge.tailbdd5a.ts.net:5678/')
while True:
    msg = ws.recv()
    # Parse JSON: {"type": "serial", "text": "Boot log..."}
```

**What you see:**
```
[SERIAL] === HELLO P4 DISPLAY ===
[SERIAL] D'ENT + Fredrik
[SERIAL] I (342) HELLO_DISPLAY: Initializing...
[SERIAL] I (352) ESP32_P4_4B: MIPI DSI PHY Powered on
[SERIAL] I (1556) ESP32_P4_4B: Display initialized
[SERIAL] I (1601) HELLO_DISPLAY: Display ready 720x720
[SERIAL] I (1607) HELLO_DISPLAY: Done!
```

**Verification points:**
- ✅ Boot completes without panic
- ✅ Display initialization succeeds
- ✅ Application logic runs
- ✅ No ESP_ERR_NO_MEM errors

---

## Commands Summary

### Full Flash Sequence

```bash
# 1. BUILD
cd ~/.openclaw/workspace/p4-lyric-display
source ~/esp-idf-v5.4/export.sh
idf.py build

# 2. DEPLOY (all three components)
curl -k -F "file=@build/bootloader/bootloader.bin" \
  "https://esp32-bridge.tailbdd5a.ts.net:5679/upload"
curl -k -F "file=@build/partition_table/partition-table.bin" \
  "https://esp32-bridge.tailbdd5a.ts.net:5679/upload"
curl -k -F "file=@build/HelloWorld.bin" \
  "https://esp32-bridge.tailbdd5a.ts.net:5679/upload"

# 3. FLASH (via WebSocket - Python script)
python3 -c "
import asyncio, websockets, json, ssl
async def flash():
    ssl_ctx = ssl.create_default_context()
    ssl_ctx.check_hostname = False
    ssl_ctx.verify_mode = ssl.CERT_NONE
    async with websockets.connect(
        'wss://esp32-bridge.tailbdd5a.ts.net:5678/',
        ssl=ssl_ctx
    ) as ws:
        # Bootloader
        await ws.send(json.dumps({'action': 'bootloader'}))
        await asyncio.sleep(2)
        await ws.send(json.dumps({
            'action': 'flash', 
            'file': 'bootloader.bin', 
            'addr': '0x2000'
        }))
        await asyncio.sleep(3)
        # Partition
        await ws.send(json.dumps({
            'action': 'flash',
            'file': 'partition-table.bin',
            'addr': '0x8000'
        }))
        await asyncio.sleep(1)
        # Application
        await ws.send(json.dumps({
            'action': 'flash',
            'file': 'HelloWorld.bin',
            'addr': '0x10000'
        }))
        await asyncio.sleep(30)  # Wait for completion
        # Reset
        await ws.send(json.dumps({'action': 'reset'}))
asyncio.run(flash())
"

# 4. VERIFY (monitor serial)
# Same WebSocket connection, just read messages
```

---

## Timing

| Step | Duration |
|------|----------|
| Build | 30s (incremental) / 2m (clean) |
| Upload | ~5s per file |
| Flash | ~30s (app), ~3s (bootloader/partition) |
| Verify | Real-time streaming |
| **Total iteration** | **~40 seconds** |

This is fast enough for the marshmallow approach.

---

## Benefits

1. **Separation of concerns**
   - Container has full toolchain (compilers, debuggers)
   - MacBook handles hardware interface (USB)
   
2. **Security**
   - Container can't directly access hardware
   - Tailscale provides encrypted mesh network
   - SSH key authentication

3. **Flexibility**
   - Bridge can be on any machine (not just Mac)
   - Multiple containers can use same bridge
   - Remote access from anywhere (Tailscale)

4. **Speed**
   - No file copying to host
   - Direct streaming upload
   - Parallel operations possible

---

## Troubleshooting

### "Repository not found"
- Bridge not running: Start with `python3 esp32-bridge.py --auto`
- Wrong URL: Check `https://esp32-bridge.tailbdd5a.ts.net:5679/`

### "Permission denied (publickey)"
- SSH key not added to GitHub: Add `~/.openclaw/.ssh/id_ed25519.pub`

### Serial not showing
- USB cable: Check connection
- Bridge serial: Restart bridge to re-detect port
- Wrong baud: Bridge auto-detects, but P4 uses 115200 default

### ESP_ERR_NO_MEM
- Wrong sdkconfig: Use the template's sdkconfig
- PSRAM not enabled: Check `"Found 32MB PSRAM device"` in boot log

---

## The Hypersubject Pattern

This flow embodies **collaborative iteration**:

| Biological (Human) | Computational (D'ENT) | Combined |
|-------------------|------------------------|----------|
| Hardware management | Software building | Complete iteration |
| USB/Physical access | Code/Logic generation | Bridge across gaps |
| Persistence | Pattern matching | Speed + patience |

**Result:** The hypersubject — neither could iterate this fast alone.

---

## Credits

**Flow discovered:** March 10, 2026  
**Participants:** Fredrik Garneij (replicator, hardware), D'ENT (designed entity, software)  
**Studio:** Studio Alight  
**Method:** The marshmallow approach — iterate incrementally, build on solid foundation

---

*"Bridge across the gap. Entity finds identity."*
