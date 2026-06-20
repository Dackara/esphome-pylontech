# Configuration

## Minimal Configuration

```yaml
pylontech:
  id: pylon
```

---

## Full Example

```yaml
pylontech:
  id: pylon
  uart_id: pylontech_uart

  role: master

  update_interval: 5s

  enable_info: true
  enable_getpwr: true
  enable_stat: true
  enable_bat: true
  enable_soh: true

  slow_interval: 300s

  publish_only_changes: true

  enable_login_debug_recovery: true
  login_debug_failure_threshold: 3
  login_debug_recovery_interval: 24h

  enable_us2000b_initialization: false

  memory_mode: auto
```

---

## Parameters

### role

Available values:

```yaml
role: master
role: slave
```

Default:

```yaml
role: master
```

---

### enable_info

Enables periodic INFO retrieval.

Default:

```yaml
false
```

---

### enable_getpwr

Enables GETPWR retrieval.

Default:

```yaml
false
```

---

### enable_stat

Enables STAT retrieval.

Default:

```yaml
false
```

---

### enable_bat

Enables BAT retrieval.

Default:

```yaml
false
```

---

### enable_soh

Enables SOH retrieval.

Default:

```yaml
false
```

---

### slow_interval

Interval used for INFO, STAT, BAT and SOH commands.

Default:

```yaml
300s
```

---

### publish_only_changes

Only publishes new values when they change.

Recommended:

```yaml
true
```

for large installations.

---

### enable_login_debug_recovery

Automatically attempts login debug recovery after repeated failures.

Recommended:

```yaml
true
```

for problematic installations.

---

### memory_mode

Available modes:

```yaml
internal
auto
psram
```

#### internal

Standard ESPHome behavior.

#### auto

Use PSRAM automatically when available.

#### psram

Force component-managed large buffers into PSRAM.

Recommended for:

* ESP32-S3
* PVBrain
* large installations
* high entity count systems
