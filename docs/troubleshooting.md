# Troubleshooting

## No Data Received

### Symptoms

* Protocol Online remains OFF
* No battery values appear
* Timeout counter increases continuously

### Checks

Verify UART wiring:

```text
ESP TX -> Battery RX
ESP RX -> Battery TX
GND -> GND
```

Verify baud rate:

```yaml
baud_rate: 115200
```

Verify UART buffer:

```yaml
rx_buffer_size: 8192
```

---

## Communication Starts Then Stops

### Symptoms

* Data appears briefly
* Protocol Online switches OFF
* Timeout counter increases

### Possible Causes

* UART wiring issue
* Noise on RS232/TTL converter
* Incorrect battery address
* Battery not in expected role

### Diagnostics

Enable:

* Last Command
* Last Error
* Response Time
* Role Validation

---

## Slave Battery Not Responding

### Symptoms

Master battery works.

Slave batteries return no data.

### Checks

Verify:

* Battery numbering
* Battery chain wiring
* Role configuration

Current slave validation is still in progress.

---

## Frequent Timeouts

### Recommended Actions

Increase UART buffer size:

```yaml
uart:
  rx_buffer_size: 8192
```

Enable publish-on-change:

```yaml
publish_only_changes: true
```

Reduce unnecessary commands:

```yaml
enable_info: false
enable_stat: false
enable_bat: false
enable_soh: false
```

until communication is stable.

---

## Login Debug Issues

Some battery firmware revisions may require Login Debug recovery.

Enable:

```yaml
enable_login_debug_recovery: true
```

---

## Memory Issues

### ESP32 Reboots

### Watchdog Resets

### Low Heap

Recommended:

```yaml
memory_mode: psram
```

and:

```yaml
psram:
```

when using ESP32-S3 hardware.

---

## Reporting Issues

Before opening an issue please provide:

* ESPHome version
* Hardware model
* Battery model
* YAML configuration
* Logs
* Role configuration
* Diagnostic entity values

This significantly reduces troubleshooting time.
