# Installation

## Requirements

### Hardware

Supported:

* ESP32
* ESP32-S2
* ESP32-S3

Recommended:

* ESP32-S3
* PSRAM enabled
* UART buffer size >= 4096 bytes

### Battery

Currently validated:

* Pylontech US2000C (Master)

Validation in progress:

* Pylontech US2000C (Slave)

Other models may work but have not yet been fully validated.

---

## Add the External Component

```yaml
external_components:
  - source:
      type: git
      url: ${GITHUB_REPOSITORY}
```

---

## UART Configuration

Example:

```yaml
uart:
  id: pylon_uart
  tx_pin: GPIO17
  rx_pin: GPIO18
  baud_rate: 115200
  rx_buffer_size: 4096
```

Recommended values:

| Parameter      | Recommended |
| -------------- | ----------- |
| baud_rate      | 115200      |
| rx_buffer_size | 4096        |

---

## Basic Component Configuration

```yaml
pylontech:
  id: pylon
  uart_id: pylon_uart
  role: master
```

---

## Verify Communication

After boot:

* Protocol Online should become ON
* No timeout counter increase
* Response time should remain stable

If communication does not start, see troubleshooting.md.
