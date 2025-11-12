.PHONY: help dev-comp dev-run dev-clean-run

help:
	@echo "ESP 32 IDF SETUP GUIDE - Available Commands"
	@echo "=================================================="
	@echo ""
	@echo " make dev-dt          Compile the device tree parser example"
	@echo " make dev-run         Compile and run the device tree parser example"
	@echo " make dev-clean-run   Clean, compile, and run the device tree parser example"
	@echo ""

dev-comp:
	gcc -lcurl main.c cJSON.c -o main 

dev-run: 
	gcc -lcurl main.c cJSON.c -o main 
	./main

dev-clean-run:
	rm -f d
	gcc -lcurl main.c cJSON.c -o main 
	./main
