# target-3-remote

`target-3` is an open-source recovery machine for shooting targets.

The server and remote communicate with Bluetooth Low Energy (BLE) protocol.
This remote program runs on small hardware such as IoT hardware.
A simple button is connected to the hardware to send the raise signal to the server.

Please refer to these repositories if you have interest in other modules of `target-3`.

* Server: https://github.com/katsuster/target-3-server
* Board: https://github.com/katsuster/target-3-board


## Requires

* Software
  * Arduino IDE
  * Boards Manager: M5Stack
* Hardware
  * M5Stamp C3


## How to use

Same as target-6-server.

* target-6-server: https://github.com/katsuster/target-6-server


## Source structures

* c3/
  * For M5Stamp C3


## LED

The RGB LED of M5Stamp C3 shows the state of the remote controller.
If several states are active at the same time, the upper one in the table is shown.

| Color  | Pattern | State |
|--|--|--|
| Red    | On (1s)              | Received NG response, timeout, or pairing failed |
| White  | On (150ms)           | Received OK response |
| Green  | Blink (3 times)      | Pairing: Succeeded |
| Yellow | On                   | Pairing: Not paired |
| Yellow | Blink (250ms on/off) | Pairing: Finding the device with the strongest RSSI |
| Green  | On                   | Connected via BLE |
| Purple | Blink (250ms on/off) | Scanning server device |


## Pairing

The remote connects to the server saved by pairing.

1. Put the remote close to the server (within about 1m).
2. Hold the button (BUTTON, BUTTON_EXT1 or BUTTON_EXT2) and power on (or reset) the remote.
3. The LED blinks yellow while pairing (about 5 seconds).
   The remote saves the server with the strongest RSSI (-50dBm or more).
4. The LED blinks green 3 times if succeeded, or shows red if failed.
   The saved server is not changed if failed.

The saved server can also be shown or changed by USB serial (115200bps).

* `peer`: Show the saved server.
* `peer set XX:XX:XX:XX:XX:XX`: Set the server address.
* `peer clear`: Clear the saved server.


## Other documents

* [GPIO](doc/gpio.md)
