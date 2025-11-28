FQBN = adafruit:samd:adafruit_matrixportal_m4
PORT = /dev/ttyACM0
compile:
	arduino-cli compile --fqbn $(FQBN) .
upload:
	arduino-cli upload -p $(PORT) --fqbn $(FQBN) .
monitor:
	arduino-cli monitor -p $(PORT)
all: compile upload monitor
