FQBN = arduino:esp32:nano_nora
PORT = /dev/ttyACM0
compile:
	arduino-cli compile --fqbn $(FQBN) .
upload:
	arduino-cli upload -p $(PORT) --fqbn $(FQBN) .
monitor:
	arduino-cli monitor -p $(PORT)
all: compile upload monitor
