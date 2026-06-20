# ESPHome Pylontech Component

[🇫🇷 Documentation Française](README.fr.md)

---

# Community & Support

🎥 **YouTube** https://youtube.com/@DackaraDemo

💬 **Discord** https://discord.gg/DAeEtv4e7k

☕ **Buy Me a Coffee** https://github.com/sponsors/Dackara

---

# Project Status

**Current Version:** v1.0.0

Current validation status:

| Hardware                   | Status          |
| -------------------------- | --------------- |
| Pylontech US2000C (Master) | ✅ Validated    |
| Pylontech US2000C (Slave)  | ⚠ In Progress   |
| Other Models               | ⚠ In Progress   |

---

# Overview

ESPHome Pylontech Component is a native ESPHome external component designed to communicate directly with Pylontech batteries through UART.

Unlike earlier implementations that focused on a limited subset of the protocol, this project aims to expose as much useful information as possible while remaining lightweight, stable and suitable for large Home Assistant deployments.

The component supports:

* Master and Slave operation
* PWRSYS monitoring
* PWR monitoring
* GETPWR monitoring
* INFO retrieval
* STAT retrieval
* BAT retrieval
* SOH retrieval
* Cell voltages
* Cell temperatures
* Cell status information
* Home Assistant integration
* Optional PSRAM optimization
* Advanced diagnostics and troubleshooting tools

---

# Features

## Fast Loop

Fast commands:

* PWRSYS
* PWR
* GETPWR

These commands provide near real-time operating data.

## Slow Loop

Slow commands:

* INFO
* STAT
* BAT
* SOH

These commands provide detailed battery information and historical statistics.

## Memory Optimization

Version 1.0.0 introduces optional memory optimization modes:

```yaml
memory_mode: internal
memory_mode: auto
memory_mode: psram
```

### internal

Standard ESPHome behavior.

### auto

Uses PSRAM automatically when available.

### psram

Forces large component-managed buffers into PSRAM when available.

This mode is specifically intended for large ESP32-S3 projects such as PVBrain where preserving internal RAM is more important than maximizing buffer access speed.

The ESPHome UART driver itself remains managed by ESPHome and is not affected by this setting.

---

# Installation

Add the external component:

```yaml
external_components:
  - source:
      type: git
      url: https://github.com/Dackara/esphome-pylontech
```

See the documentation folder for complete configuration examples.

---

# Diagnostics

The component includes several optional diagnostic entities.

These entities are primarily intended for:

* protocol validation
* troubleshooting
* development
* advanced debugging

Most users do not need to enable them.

---

# Acknowledgements

Special thanks to KJM whose previous ESPHome implementation served as the primary foundation and reference for this project.

Additional thanks to Functionpointer for earlier work on Pylontech protocol integration and UART handling concepts.

---

# Development Notes

This project was developed by Dackara.

Parts of this project were reviewed and optimized with the assistance of AI-based development tools including ChatGPT, Codex and Claude.

All design decisions, implementation choices and validation remain the responsibility of the project maintainer.

---

# Disclaimer

This project is not affiliated with Pylontech.

Use at your own risk.
