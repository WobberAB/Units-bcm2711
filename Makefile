CC=gcc
CFLAGS=-Wall -Wextra -pedantic -Wstrict-prototypes -Wno-unused-parameter -Wno-overlength-strings
BUILD_DIR=build
SOURCE_DIR=src
OBJECT_DIR=obj
LIBS=-I/usr/include/mariadb -lmariadb -pthread -ldl -lm -lpthread -lz -lpigpiod_if2

$(BUILD_DIR)/gpio2sql: $(OBJECT_DIR)/main.o $(OBJECT_DIR)/config.o  $(OBJECT_DIR)/fileUtils.o $(OBJECT_DIR)/gpio.o $(OBJECT_DIR)/invert.o $(OBJECT_DIR)/pid.o $(OBJECT_DIR)/prefix.o $(OBJECT_DIR)/tables.o $(OBJECT_DIR)/update.o $(OBJECT_DIR)/bwlog.o
	$(CC) -o $(BUILD_DIR)/gpio2sql $(OBJECT_DIR)/*.o $(CFLAGS) $(LIBS)

$(BUILD_DIR)/gpio-req-update: $(SOURCE_DIR)/gpio-req-update.c $(OBJECT_DIR)/pid.o
	$(CC) -o $(BUILD_DIR)/gpio-req-update $(SOURCE_DIR)/gpio-req-update.c $(OBJECT_DIR)/pid.o

$(OBJECT_DIR)/main.o: $(SOURCE_DIR)/main.c
	$(CC) -c $(SOURCE_DIR)/main.c -o $(OBJECT_DIR)/main.o

$(OBJECT_DIR)/tables.o: $(SOURCE_DIR)/tables.c
	$(CC) -c $(SOURCE_DIR)/tables.c -o $(OBJECT_DIR)/tables.o

$(OBJECT_DIR)/config.o: $(SOURCE_DIR)/config.c
	$(CC) -c $(SOURCE_DIR)/config.c -o $(OBJECT_DIR)/config.o

$(OBJECT_DIR)/pid.o: $(SOURCE_DIR)/pid.c
	$(CC) -c $(SOURCE_DIR)/pid.c -o $(OBJECT_DIR)/pid.o

$(OBJECT_DIR)/prefix.o: $(SOURCE_DIR)/prefix.c
	$(CC) -c $(SOURCE_DIR)/prefix.c -o $(OBJECT_DIR)/prefix.o

$(OBJECT_DIR)/update.o: $(SOURCE_DIR)/update.c
	$(CC) -c $(SOURCE_DIR)/update.c -o $(OBJECT_DIR)/update.o

$(OBJECT_DIR)/invert.o: $(SOURCE_DIR)/invert.c
	$(CC) -c $(SOURCE_DIR)/invert.c -o $(OBJECT_DIR)/invert.o

$(OBJECT_DIR)/gpio.o: $(SOURCE_DIR)/gpio.c
	$(CC) -c $(SOURCE_DIR)/gpio.c -o $(OBJECT_DIR)/gpio.o

$(OBJECT_DIR)/bwlog.o: $(SOURCE_DIR)/bwlog.c
	$(CC) -c $(SOURCE_DIR)/bwlog.c -o $(OBJECT_DIR)/bwlog.o

$(OBJECT_DIR)/fileUtils.o: $(SOURCE_DIR)/fileUtils.c
	$(CC) -c $(SOURCE_DIR)/fileUtils.c -o $(OBJECT_DIR)/fileUtils.o


install:
	mkdir -p /usr/wobber/gpio2sql/binaries
	mkdir -p /mnt/ramdisk/log
	cp $(BUILD_DIR)/gpio2sql /usr/wobber/gpio2sql/binaries
	cp $(BUILD_DIR)/gpio-req-update /usr/wobber/gpio2sql/binaries
	chown -R mysql:mysql /usr/wobber/gpio2sql
	chown -R mysql:mysql /mnt/ramdisk/log
	mkdir -p /usr/wobber/gpio2sql
	cp -rvn conf/* /usr/wobber/gpio2sql/

remove:
	rm -rf /mnt/ramdisk/log/gpio2sql.log
	rm -rf /usr/wobber/gpio2sql

service:
	cp -v systemd/gpio2sql.service /etc/systemd/system/gpio2sql.service
	systemctl daemon-reload
	systemctl enable gpio2sql
	service gpio2sql restart
	service gpio2sql status

all: $(BUILD_DIR)/gpio2sql $(BUILD_DIR)/gpio-req-update install service

clean:
	rm -f $(BUILD_DIR)/gpio2sql
	rm -f $(OBJECT_DIR)/*.o

dist: clean
	tar cvzf ../gpio2sql-13RC2-`date +%F%H%M`-`hostname`.tar.gz ../gpio2sql
