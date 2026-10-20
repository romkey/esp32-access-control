# Configuration

Every example needs `secrets.yaml` next to it (copy `examples/secrets.yaml.example`).
Shared settings are in `examples/common/base.yaml`: `esphome: project:`, esphome-lib
diagnostics and restart, encrypted `api:` and `ota:` (one key), fallback hotspot with
captive portal, safe mode and factory reset buttons, and `web_server` v3 with digest
auth and OTA upload off. Secrets: `wifi_ssid`, `wifi_password`, `api_encryption_key`,
`fallback_ap_password`, `web_server_username`, `web_server_password`.

## `examples/access-control.yaml`

The door controller for the ESP32-S3 board. Also needs secrets `push_auth_token` and
`syslog_server`. Reader 1 uses D0 = GPIO18, D1 = GPIO17, LED = GPIO16, beep = GPIO15
(both active low). Door relays are **placeholders** on GPIO38/39 (J1); GPIO19/20 are the
S3's USB pins and must not be used.

Using the packages from your own config: add `external_components` pointing at this repo
(for `components/`), include `packages/access-door.yaml`, and set the substitutions below.

## `packages/access-door.yaml`

Bundle that includes the five packages below. Set their required substitutions:
`door_relay_1_pin`, `door_relay_2_pin`, `wiegand_d0_pin`, `wiegand_d1_pin`,
`wiegand_led_pin`, `wiegand_beep_pin`.

| Package | Provides | Main substitutions (default) |
|---------|----------|------------------------------|
| `push-auth.yaml` | `push_auth_cfg`; token auth for HTTP endpoints | `push_auth_header` (Authorization), `push_auth_alt_header` (X-Push-Token) |
| `access-db.yaml` | `access_db_cfg`; tag database in a LittleFS partition; lookup result entities | `access_db_path` (/access-db), `access_db_max_body_size` (131072), `access_db_partition_label` (storage), `access_db_mount_path` (/storage) |
| `access-site.yaml` | site name, syslog (online/door-opened), and an `access_site_unlock_path` endpoint that logs a person's unlock (it does not fire the relay); secret `syslog_server` (IPv4 address or hostname; IPv6 not supported) | `access_site_controller_name`, `access_site_syslog_port` (514), `access_site_hostname`, `access_site_unlock_path` (/access-unlock) |
| `door-relays.yaml` | `door_relay_1/2` switches, `door_unlock_pulse` script | `door_relay_count` (1), `door_relay_inverted` (false), `door_relay_pulse_ms` (3000) |
| `wiegand-reader.yaml` | Wiegand reader, LED and beeper, last tag/key, facility and card | `wiegand_led_inverted`, `wiegand_beep_inverted` (false) |

Every entity name is also a substitution (`*_name`); see each file's header. Packages
reference each other's ids (`push_auth_cfg`, `access_db_cfg`, `door_unlock_pulse`, ...),
so include them together via `access-door.yaml`.

Uploads must finish within 30 seconds (PUT; receive timeouts are retried until then) or are rejected with 408; a connection that drops mid-upload is cleaned up and the status shows `connection_lost` or `upload_stalled`. The access database and unlock endpoints take POST bodies as `application/json` (ESPHome hands form-encoded bodies to the handler differently, so a form content type is rejected).

A syslog hostname is resolved in the background; until it is known, syslog messages are held (the latest 8) and sent when it resolves, so a grant never waits on DNS. The address is looked up again after a reconnect or a failed send.

A successful tag lookup publishes the matched user, pulses the door relay and logs to syslog.

## `examples/simple.yaml`

A bring-up config: LED on GPIO8, reader LED/beeper switches, Wiegand reader 1, and text
sensors `Last Tag`, `Last Tag Raw` and `Last Tag H10301` (26-bit frames passing parity).
A good read runs the `acknowledge` script (150 ms beep, 500 ms LED flash). Reader 2 is unused.

## C++ components (`components/`)

`access_db` (sensor, text_sensor, binary_sensor), `access_site` (text_sensor),
`access_syslog` (with `send_online` / `send_door_opened`), `push_auth` (sensor, text_sensor).
All options are used by `examples/access-control.yaml`, which CI compiles.

```bash
docker compose run --rm validate
docker compose run --rm build
```
