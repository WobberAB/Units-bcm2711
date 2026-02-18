GPIO to MySQL Daemon for Raspberry Pi 3 and 4

A daemon that bridges Raspberry Pi 3 and 4 GPIO pins with a MySQL database, enabling database-driven GPIO control and monitoring.

## Features

- ✅ **Full Raspberry Pi 3 & 4 support** using pigpiod
- ✅ **GPIO input/output** with pull-up/down resistor configuration
- ✅ **Hardware PWM** on pins 12/13/19 (0-100% duty cycle)
- ✅ **Edge detection** for interrupt-driven updates
- ✅ **Glitch filtering** (debounce) support
- ✅ **MySQL integration** for remote monitoring and control
- ✅ **Security hardened** with SQL injection protection
- ✅ **Database-driven configuration** for all GPIO pins 
  
## System Requirements

### Software
- pipgpiod
- MySQL/MariaDB server
- GCC compiler and build tools

## Installation

### 1. Install Dependencies

```bash
sudo apt-get update
sudo apt-get install -y libpigpiod-if-dev libpigpiod-if2-1t64 libmariadb-dev build-essential libmariadb-dev-compat mariadb-server

```

### 2. Configure MySQL
From shell, run:

```bash
sudo mysql -u root
```

Then copy, edit and paste the following
```sql
CREATE DATABASE ioadc;
CREATE USER 'defaultuser'@'localhost' IDENTIFIED BY 'defaultpassword';
GRANT SELECT, INSERT, UPDATE ON ioadc.* TO 'defaultuser'@'localhost';
FLUSH PRIVILEGES;
```
### 3. pigpiod missing from Trixie(13)
It seems as if the team has removed pigpiod from repos. fetch and compile/install with
```
sudo apt install python3-distutils-extra -y

wget https://github.com/joan2937/pigpio/archive/master.zip
unzip master.zip
cd pigpio-master
make
sudo make install
```

### 4. Compile and Install

```bash
# Clone or extract source code
cd /path/to/source

# Compile and install
# (this also installs default configuration files and systemd service)

# for new install
sudo make all

# for upgrade
sudo make upgrade
```

## Usage

### Starting the Daemon

```bash
# systemd (see systemd section below)
sudo systemctl start units-bcm2711
```

### GPIO Configuration

GPIO pins are configured via the `gpio-conf` table in MySQL:

```sql
-- Configure pin 17 as output
UPDATE `gpio-conf` SET 
    `direction` = 0,    -- 0=output, 1=input, 2=pwm
    `enabled` = 1       -- 1=enabled, 0=disabled
WHERE `pin` = 17;

-- Configure pin 22 as input with pull-up
UPDATE `gpio-conf` SET 
    `direction` = 1,    -- input
    `pullup` = 2,       -- 0=off, 1=pull-down, 2=pull-up
    `interrupt` = 4,    -- 0=disabled, 1=falling, 2=rising, 4=both
    `enabled` = 1
WHERE `pin` = 22;

-- Configure pin 12 as PWM output
UPDATE `gpio-conf` SET 
    `direction` = 2,    -- PWM
    `enabled` = 1
WHERE `pin` = 12;
```

### Controlling GPIO

Control GPIO pins via the `gpio` table:

```sql
-- Turn pin 17 HIGH
UPDATE `gpio` SET `req` = 1 WHERE `pin` = 17;

-- Turn pin 17 LOW
UPDATE `gpio` SET `req` = 0 WHERE `pin` = 17;

-- Set PWM on pin 12 to 50%
UPDATE `gpio` SET `req` = 50 WHERE `pin` = 12;

-- Read current value of pin 22
SELECT `value` FROM `gpio` WHERE `pin` = 22;
```

## Prefix
The address prefix is for compatibility of Wobber UNITS
and are derived from the two first letters + the last letter of your hostname so hostname 'debian' would generate the prefix 'den'.

## Database Schema

### `gpio-conf` Table
Configuration for each GPIO pin:

| Column | Type | Description |
|--------|------|-------------|
| `address` | VARCHAR(20) | Device identifier (e.g., 'raygpio') |
| `pin` | INT(2) | GPIO pin number (0-27) |
| `direction` | INT(2) | 0=output, 1=input, 2=pwm |
| `pullup` | INT(2) | 0=off, 1=pull-down, 2=pull-up |
| `interrupt` | INT(2) | 0=disabled, 1=falling, 2=rising, 4=both |
| `inverted` | INT(2) | 0=normal, 1=inverted logic |
| `glitch` | INT(2) | Debounce time in microseconds (-1001=disabled) |
| `enabled` | INT(2) | 0=disabled, 1=enabled |

### `gpio` Table
Current state and control for each pin:

| Column | Type | Description |
|--------|------|-------------|
| `address` | VARCHAR(20) | Device identifier |
| `pin` | INT(2) | GPIO pin number |
| `value` | INT(1) | Current pin value (-1001=unassigned, 0=low, 1=high) |
| `req` | INT(2) | Requested value (-1001=no request, 0-1 for digital, 0-100 for PWM) |
| `text` | VARCHAR(100) | Optional description |
| `timestamp` | BIGINT(20) | Last update timestamp (microseconds) |

# View logs
sudo journalctl -u units-bcm2711 -f
```

**Note**: Pins 2 and 3 (I2C) are skipped by default in configuration.

## Examples

### LED Control

```sql
-- Setup 17 as output
UPDATE `gpio-conf` SET `direction` = 0, `enabled` = 1 WHERE `pin` = 17;

-- Turn on
UPDATE `gpio` SET `req` = 1 WHERE `pin` = 17;

-- Turn off
UPDATE `gpio` SET `req` = 0 WHERE `pin` = 17;
```

### Button Input with Pull-up

```sql
-- Setup with pull-up resistor and interrupt
UPDATE `gpio-conf` SET 
    `direction` = 1,
    `pullup` = 2,
    `interrupt` = 4,
    `enabled` = 1 
WHERE `pin` = 22;

-- Read value (1=not pressed, 0=pressed)
SELECT `value` FROM `gpio` WHERE `pin` = 22;
```

### PWM LED Dimming

```sql
-- Setup
UPDATE `gpio-conf` SET `direction` = 2, `enabled` = 1 WHERE `pin` = 18;

-- Set brightness to 25%
UPDATE `gpio` SET `req` = 25 WHERE `pin` = 18;

-- Set brightness to 75%
UPDATE `gpio` SET `req` = 75 WHERE `pin` = 18;
```
### MySQL connection fails

```bash
# Test MySQL connection
mysql -u defaultuser -p -h localhost ioadc

# Check configuration files
cat /etc/units/bcm2711/mysql/*.cnf
```

## Performance

- **CPU Usage**: ~1-2% idle, +5-10% per active PWM pin
- **Memory Usage**: ~5-8MB
- **Update Latency**: <1ms for GPIO operations
- **Database Polling**: Configurable (default 1 second)

## Security Notes

1. **Secure MySQL credentials**: Configuration files should be chmod 600
2. **SQL injection protection**: Built-in input validation
3. **Buffer overflow protection**: All inputs bounds-checked
4. **Network security**: Use MySQL over localhost or encrypted connections

## Architecture
```
MySQL Database ← → units-bcm2711 daemon ← → pigpiod ← → Hardware
```

## Limitations

1. **Raspberry Pi 3 and 4 only**: Code uses pigpiod, specific to Pi 3 and 4

## License
GPLv2 

## Contributing
Feel free to contribute

## Support

For issues:
1. Check system logs: `journalctl -u units-bcm2711`
2. Test MySQL connection
3. Check if pigpiod is up and running
4. Check file permissions on `/etc/units/bcm2711/`
