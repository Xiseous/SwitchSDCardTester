# SD Card Read Tester for Nintendo Switch

A Hekate payload that performs comprehensive SD card read testing with sequential and butterfly patterns, measuring latency and detecting bad blocks. Features a touch-enabled LVGL GUI.

## Features

- **Sequential Read Test**: Reads blocks from start to end
- **Butterfly Read Test**: Alternating low/high sector reads (tests seek performance)
- **Latency Measurement**: Min/max/average latency per read operation
- **Bad Block Detection**: Identifies read failures and slow blocks (>5ms)
- **Fast/Full Modes**: Quick 4GB tests or full card verification
- **Touch-enabled GUI**: Modern LVGL interface with progress bars and buttons

## Building

### Prerequisites
- devkitARM (Nintendo Switch homebrew toolchain)
- Set `DEVKITARM` environment variable

### Build
```bash
cd GUI
make clean && make
# Output: output/SDCardTester_GUI.bin
```

## Usage

1. Copy `SDCardTester.bin` to `/bootloader/payloads/` on your SD card
2. Boot into Hekate
3. Select Payloads → SDCardTester
4. Choose a test mode:
   - **Sequential 4GB** - Quick sequential read test
   - **Butterfly 4K iter** - Quick random access test
   - **Full Sequential** - Test entire card sequentially
   - **Full Butterfly** - Full random access test
   - **All Fast/Full** - Combined tests

## Test Results

After running tests, a results popup will display:
- Blocks tested and read errors
- Latency statistics (min/max/avg in microseconds)
- Slow block count (blocks >5ms response time)
- Pass/Fail status

## Credits

Based on:
- Hekate by CTCaer
- Nyx GUI (LVGL implementation)
- TegraExplorer by SuchMemeManySkill

## License

GNU General Public License v2.0
