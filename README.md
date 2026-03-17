# SpectroPhoto Mechatronics Firmware

Arduino Uno R3 firmware for the spectrophotometer mechatronics subsystem.

The sketch controls:
- the carousel stepper
- the home and door inputs
- lamp and heater MOSFET outputs
- the SCPI-like serial command interface

## Project Layout

The code is still organized as an Arduino sketch split across multiple `.ino` files:
- `Spectrophoto8.ino` - startup, globals, and main loop
- `SCPI.ino` - command registration and handlers
- `Carousel.ino` - carousel initialization and motion
- `Temperature.ino` - NTC temperature readout
- `EEPROM.ino` - configuration storage and CRC validation

## VS Code Workflow

Open this folder in VS Code:

`C:\Users\Marin\Documents\Arduino\Spectrophoto8`

Recommended extensions:
- `ms-vscode.cpptools`
- `vsciot-vscode.vscode-arduino`

Available VS Code tasks:
- `Arduino: Compile`
- `Arduino: Upload`
- `Arduino: Monitor`

The workspace is configured for:
- board: `arduino:avr:uno`
- default serial port: `COM5`
- serial baud rate: `9600`

Run tasks from:

`Terminal -> Run Task`

## Command Line Build

Compile:

```powershell
& 'C:\Program Files\Arduino IDE\resources\app\lib\backend\resources\arduino-cli.exe' compile --fqbn arduino:avr:uno 'C:\Users\Marin\Documents\Arduino\Spectrophoto8'
```

Upload:

```powershell
& 'C:\Program Files\Arduino IDE\resources\app\lib\backend\resources\arduino-cli.exe' upload -p COM5 --fqbn arduino:avr:uno 'C:\Users\Marin\Documents\Arduino\Spectrophoto8'
```

Monitor:

```powershell
& 'C:\Program Files\Arduino IDE\resources\app\lib\backend\resources\arduino-cli.exe' monitor -p COM5 --config baudrate=9600
```

## Notes

- If Windows assigns a different COM port, update the port selection in the VS Code task prompt.
- The project currently builds cleanly for Arduino Uno R3 with `arduino-cli`.
- As the firmware grows, a future cleanup step will be to migrate shared declarations into `.h` files and move logic into `.cpp` modules.
