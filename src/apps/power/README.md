# Power link

The application has two compile-time roles:

- `power_send`: reads an Eastron SDM120 over Modbus RTU and transmits authenticated LoRa packets.
- `power_receive`: verifies LoRa packets and emits newline-delimited JSON for an ESP.

Build both images with `./build_power.sh`, or pass `send` or `receive`.

## Sender wiring

The SDM120 must be connected through a 3.3 V RS-485 transceiver, preferably an isolated module. Never connect its RS-485 terminals directly to the STM32 UART.

| Heltec | RS-485 transceiver |
|---|---|
| PA9 / TX | DI |
| PA10 / RX | RO |
| PA0 | DE and active-low RE tied together |
| GND | logic-side GND |

The transceiver bus side connects A, B and, when required by the module and installation, reference ground to the SDM120. Work on the mains side must be performed by a qualified electrician.

The firmware expects the SDM120 factory communication settings: address 1, 2400 baud, 8N1. It reads input registers 0 through 13 with Modbus function 04, covering voltage, current and active power.

## Receiver-to-ESP wiring

| Heltec receiver | ESP |
|---|---|
| PA9 / TX | RX, 3.3 V logic |
| GND | GND |
| PA10 / RX | TX, optional |

The receiver UART runs at 115200 baud, 8N1. Each accepted packet produces one JSON object followed by `\n`:

```json
{"device":1,"boot_id":123,"seq":42,"modbus_ok":true,"power_w":487,"voltage_v":230.4,"current_ma":2114,"rssi":-83,"snr":8}
```

`modbus_ok:false` means that the sender did not receive or validate an SDM120 response. Such status packets are still authenticated, but zero values in them are not used to enter the reduced-rate night mode.

## Authentication key

The key in `HeltecLoRa151/power_protocol.c` is a development key. Replace it with the same randomly generated 16-byte secret in both firmware images before deployment, and do not commit the production key.
