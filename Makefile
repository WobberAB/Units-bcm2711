CC=gcc
CFLAGS=-Wall -Wextra -pedantic -Wstrict-prototypes -Wno-unused-parameter -Wno-overlength-strings
BUILD_DIR=build
SOURCE_DIR=src
OBJECT_DIR=obj
LIBS=-I/usr/include/mariadb -lmariadb -pthread -ldl -lm -lpthread -lz -lpigpiod_if2

$(BUILD_DIR)/units-bcm2711: $(OBJECT_DIR)/main.o $(OBJECT_DIR)/config.o  $(OBJECT_DIR)/fileUtils.o $(OBJECT_DIR)/gpio.o $(OBJECT_DIR)/invert.o $(OBJECT_DIR)/pid.o $(OBJECT_DIR)/prefix.o $(OBJECT_DIR)/tables.o $(OBJECT_DIR)/update.o
	$(CC) -o $(BUILD_DIR)/units-bcm2711 $(OBJECT_DIR)/*.o $(CFLAGS) $(LIBS)

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

$(OBJECT_DIR)/fileUtils.o: $(SOURCE_DIR)/fileUtils.c
	$(CC) -c $(SOURCE_DIR)/fileUtils.c -o $(OBJECT_DIR)/fileUtils.o


install:
	mkdir -p /etc/units/bcm2711/
	chown -R mysql:mysql /etc/units/bcm2711
	mkdir -p /mnt/ramdisk/log
	chown -R mysql:mysql /mnt/ramdisk/log
	cp $(BUILD_DIR)/units-bcm2711 /usr/bin/
	cp -rvn conf/* /etc/units/bcm2711/

remove:
	rm -rf /mnt/ramdisk/log/units-bcm2711.log
	rm -rf /usr/bin/units-bcm2711
	
service:
	cp -v systemd/units-bcm2711.service /etc/systemd/system/units-bcm2711.service
	systemctl daemon-reload
	systemctl enable units-bcm2711
	service units-bcm2711 restart
	service units-bcm2711 status

all: $(BUILD_DIR)/units-bcm2711 stop install service

stop:
	systemctl stop units-bcm2711

clean:
	rm -f $(BUILD_DIR)/*
	rm -f $(OBJECT_DIR)/*

dist: clean
	tar cvzf ../units-bcm2711-13RC2-`date +%F%H%M`-`hostname`.tar.gz ../units-bcm2711
