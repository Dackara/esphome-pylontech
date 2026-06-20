# Protocol Overview

## Important Notice

This document is intended only as a high-level overview.

It does not attempt to fully document the proprietary Pylontech protocol.

---

## Design Goals

The component was designed around four objectives:

1. Stability
2. Low CPU usage
3. Low RAM usage
4. Home Assistant integration

---

## Fast Loop

The fast loop is responsible for frequently changing values.

Commands:

```text
PWRSYS
PWR
GETPWR
```

Typical information:

* Voltage
* Current
* Power
* Capacity
* State of Charge
* Cell voltages
* Cell temperatures

---

## Slow Loop

The slow loop retrieves information that changes infrequently.

Commands:

```text
INFO
STAT
BAT
SOH
```

Typical information:

* Firmware information
* Manufacturer information
* Alarm information
* Protection information
* State of Health

---

## Command Scheduling

The component uses a non-blocking command scheduler.

Commands are executed sequentially.

Each command must complete before the next command starts.

This prevents protocol collisions and improves stability.

---

## Publish On Change

When enabled:

```yaml
publish_only_changes: true
```

entities are only published when their value changes.

Benefits:

* Lower Home Assistant database usage
* Reduced network traffic
* Reduced ESPHome workload

---

## Memory Optimization

Version 1.0.0 introduces optional memory modes.

### internal

Default behavior.

### auto

Automatic PSRAM usage when available.

### psram

Large component-managed buffers are explicitly allocated in PSRAM.

This mode is primarily intended for:

* ESP32-S3
* PVBrain
* High entity count installations

---

## Diagnostics

Several diagnostic entities are available for protocol validation and troubleshooting.

These entities were primarily created during development and testing of the component.

Most users do not need them for daily operation.
