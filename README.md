# SD Card Read Tester for Nintendo Switch

A Hekate payload that performs comprehensive SD card read testing with sequential and butterfly patterns, measuring latency and detecting bad blocks.

## Features

- **Sequential Read Test**: Reads blocks from start to end
- **Butterfly Read Test**: Alternating low/high sector reads (tests seek performance)
- **Latency Measurement**: Min/max/average latency per read operation
- **Bad Block Detection**: Identifies read failures and slow blocks (>5ms)
- **Fast/Full Modes**: Quick 512MB tests or full card verification

## Versions

### Text Version (`Text/`)
Simple text-based interface using `gfx_printf()` and menu system.
- Lightweight and fast
- Based on CommonProblemResolver patterns

### GUI Version (`GUI/`) 
Full graphical interface using LVGL (LittlevGL).
- Buttons, progress bars, message boxes
- Based on Nyx (Hekate's GUI)

## Building

### Prerequisites
- devkitARM (Nintendo Switch homebrew toolchain)
- Set `DEVKITARM` environment variable

### Build Text Version
```bash
cd Text
make clean && make
# Output: output/SDCardTester.bin
```

### Build GUI Version
```bash
cd GUI
make clean && make
# Output: output/SDCardTester_GUI.bin
```

## Usage

1. Copy the `.bin` file to `/bootloader/payloads/` on your SD card
2. Boot into Hekate
3. Select Payloads → SDCardTester
4. Choose a test mode:
   - **Fast Sequential (512 MB)** - Quick sequential read test
   - **Fast Butterfly (512 iter)** - Quick random access test
   - **Full Sequential** - Test entire card sequentially
   - **Full Butterfly** - Full random access test
   - **Run All** - Combined tests

## Test Output

```
=== SD Card Read Tester Results ===

Card: 128 GB (UHS-I SDR104)

-- Sequential Read Test --
  Blocks Tested: 1048576
  Read Errors:   0
  Slow Blocks:   3 (>5ms)
  Latency - Min: 142 us, Max: 4521 us, Avg: 1823 us

-- Butterfly Read Test --
  Blocks Tested: 1024
  Read Errors:   0
  Slow Blocks:   12 (>5ms)
  Latency - Min: 234 us, Max: 8934 us, Avg: 3456 us

[PASSED] - SD card passed all read tests!
```

## Credits

Based on:
- Hekate by CTCaer
- CommonProblemResolver by Team Neptune
- TegraExplorer by SuchMemeManySkill

## License

GNU General Public License v3.0
