# Makefile for Arduino CLI projects
#
# NOTE on .DELETE_ON_ERROR:
# If compile fails partway through, make deletes the target .bin so a half-written file
# doesn’t appear "up-to-date". This won’t catch: PHONY targets, tools that write/move elsewhere,
# or silent corruption (exit 0). For release/final builds, consider: make clean

# Quiet recursive make "Entering/Leaving directory" lines
MAKEFLAGS += --no-print-directory

JQ := $(shell command -v jq 2>/dev/null)
ifeq ($(JQ),)
$(error "jq not found, please install jq")
endif

# ----- user settings -----
CONFIG := ./arduino-cli.yaml
BUILD  := build
BAUD   ?= 115200
FQBN   ?= esp32:esp32:esp32s3
DTO    ?= 100ms
LFRMT  ?=                # pass `LFRMT=--json` to get JSON; `make list LFRMT=--json`
PORT   ?=                # pass `PORT=/dev/ttyXXX`; e.g. `make upload S=Foo PORT=/dev/ttyUSB0`
CLI_V  ?=                # pass `CLI_V=-v`; e.g. `make compile S=Foo CLI_V=-v`
OTHER_COMPILE_PARAMS ?=              # pass `OTHER_COMPILE_PARAMS=xx` other compile parameters such as --only-compilation-database

CLI := arduino-cli $(CLI_V) --config-file "$(CONFIG)"

# --- sketch selection: allow short var S or long SKETCH; strip trailing slash ---
SK      := $(strip $(or $(S),$(SKETCH)))
SKETCH  := $(patsubst %/,%,$(SK))

.PHONY: help init uninit lib.help list l compile upload monitor clean c u m cl
.DEFAULT_GOAL := help

INDENT := %*s
INDENT_ZR0 := 0	         # spaces to indent for right-aligned Usage/Targets help text
INDENT_PAD := 2	         # spaces to indent in Targets help text
DESC_PAD := 5		     # spaces between longest targets and description in help text

define indent
	@printf '$(INDENT)%s\n' $(1) '' $(2)
endef

define check-sketch
	@test -n "$(SKETCH)" || { echo "Error: set S=<SketchDir> (or SKETCH=)"; exit 1; }
endef

help h H: ## `make h` Show this help
	$(call indent, $(INDENT_ZR0), "Usage:")
	$(call indent, $(INDENT_PAD), "make help")
	$(call indent, $(INDENT_PAD), "make <target> {SKETCH|S}=<SketchDir>")
	$(call indent, $(INDENT_ZR0), "Examples:")
	$(call indent, $(INDENT_PAD), "make compile  SKETCH=Simple_Button_Control");
	$(call indent, $(INDENT_PAD), "make c S=Simple_Button_Control");
	$(call indent, $(INDENT_ZR0), "")
	$(call indent, $(INDENT_ZR0), "Notes:")
	$(call indent, $(INDENT_PAD), "- <SketchDir> must be a subdir containing <SketchDir>/<SketchDir>.ino")
	$(call indent, $(INDENT_PAD), "- Trailing '/' after the sketch name is accepted")
	$(call indent, $(INDENT_ZR0), "")
	$(call indent, $(INDENT_ZR0), "Targets:")
	@awk -v indent_pad=$(INDENT_PAD) -v desc_pad=$(DESC_PAD) -f support/target-list.awk $(lastword $(MAKEFILE_LIST))

# esp32:32 3.3.2 won't compile because mbedtls_md5_xxx_ret are not defined:
#    wink@3900x 25-10-31T19:19:08.864Z:~/data/prgs/PD-Stepper/Software (wink-arduino-cli-2)
#    $ make c S=PD_Stepper_Web_Server/
#    arduino-cli  --config-file "./arduino-cli.yaml" compile -b "esp32:esp32:esp32s3" --build-path "build" "./PD_Stepper_Web_Server" ; \
#
#    /home/wink/data/prgs/PD-Stepper/Software/shared/libraries/ESPAsyncWebServer/src/WebAuthentication.cpp: In function 'bool getMD5(uint8_t*, uint16_t, char*)':
#    /home/wink/data/prgs/PD-Stepper/Software/shared/libraries/ESPAsyncWebServer/src/WebAuthentication.cpp:74:3: error: 'mbedtls_md5_starts_ret' was not declared in this scope; did you mean 'mbedtls_md5_starts'?
#       74 |   mbedtls_md5_starts_ret(&_ctx);
#          |   ^~~~~~~~~~~~~~~~~~~~~~
#          |   mbedtls_md5_starts
#    /home/wink/data/prgs/PD-Stepper/Software/shared/libraries/ESPAsyncWebServer/src/WebAuthentication.cpp:75:3: error: 'mbedtls_md5_update_ret' was not declared in this scope; did you mean 'mbedtls_md5_update'?
#       75 |   mbedtls_md5_update_ret(&_ctx, data, len);
#          |   ^~~~~~~~~~~~~~~~~~~~~~
#          |   mbedtls_md5_update
#    /home/wink/data/prgs/PD-Stepper/Software/shared/libraries/ESPAsyncWebServer/src/WebAuthentication.cpp:76:3: error: 'mbedtls_md5_finish_ret' was not declared in this scope; did you mean 'mbedtls_md5_finish'?
#       76 |   mbedtls_md5_finish_ret(&_ctx, _buf);
#          |   ^~~~~~~~~~~~~~~~~~~~~~
#          |   mbedtls_md5_finish
#
#    Used library      Version Path
#    WiFi              3.3.2   /home/wink/data/prgs/PD-Stepper/Software/shared/data/packages/esp32/hardware/esp32/3.3.2/libraries/WiFi
#    Networking        3.3.2   /home/wink/data/prgs/PD-Stepper/Software/shared/data/packages/esp32/hardware/esp32/3.3.2/libraries/Network
#    ESPAsyncWebServer 3.1.0   /home/wink/data/prgs/PD-Stepper/Software/shared/libraries/ESPAsyncWebServer
#    FS                3.3.2   /home/wink/data/prgs/PD-Stepper/Software/shared/data/packages/esp32/hardware/esp32/3.3.2/libraries/FS
#    AsyncTCP          1.1.4   /home/wink/data/prgs/PD-Stepper/Software/shared/libraries/AsyncTCP
#    TMC2209           10.1.1  /home/wink/data/prgs/PD-Stepper/Software/shared/libraries/TMC2209
#    Preferences       3.3.2   /home/wink/data/prgs/PD-Stepper/Software/shared/data/packages/esp32/hardware/esp32/3.3.2/libraries/Preferences
#    Wire              3.3.2   /home/wink/data/prgs/PD-Stepper/Software/shared/data/packages/esp32/hardware/esp32/3.3.2/libraries/Wire
#
#    Used platform Version Path
#    esp32:esp32   3.3.2   /home/wink/data/prgs/PD-Stepper/Software/shared/data/packages/esp32/hardware/esp32/3.3.2
#    Error during build: exit status 1
#    make[1]: *** [Makefile:110: buildit] Error 1
#    make: *** [Makefile:114: c] Error 2
#    wink@3900x 25-10-31T19:19:34.749Z:~/data/prgs/PD-Stepper/Software (wink-arduino-cli-2)

# esp32:esp32@2.0.10 does compile!
#    wink@3900x 25-10-31T19:26:03.932Z:~/data/prgs/PD-Stepper/Software (wink-arduino-cli-2)
#    $ make c S=PD_Stepper_Web_Server/
#    arduino-cli  --config-file "./arduino-cli.yaml" compile -b "esp32:esp32:esp32s3" --build-path "build" "./PD_Stepper_Web_Server" ; \
#
#    Sketch uses 805241 bytes (61%) of program storage space. Maximum is 1310720 bytes.
#    Global variables use 44248 bytes (13%) of dynamic memory, leaving 283432 bytes for local variables. Maximum is 327680 bytes.
#    wink@3900x 25-10-31T19:26:24.774Z:~/data/prgs/PD-Stepper/Software (wink-arduino-cli-2)


# esp32:esp32@2.0.14 does compile!
#    wink@3900x 25-10-31T19:29:10.916Z:~/data/prgs/PD-Stepper/Software (wink-arduino-cli-2)
#    $ make c S=PD_Stepper_Web_Server/
#    arduino-cli  --config-file "./arduino-cli.yaml" compile -b "esp32:esp32:esp32s3" --build-path "build" "./PD_Stepper_Web_Server" ; \
#
#    Sketch uses 809937 bytes (61%) of program storage space. Maximum is 1310720 bytes.
#    Global variables use 44324 bytes (13%) of dynamic memory, leaving 283356 bytes for local variables. Maximum is 327680 bytes.
#    wink@3900x 25-10-31T19:29:29.689Z:~/data/prgs/PD-Stepper/Software (wink-arduino-cli-2)

init: ## `make init` One-time: init config, install core + libs
	$(CLI) config init
	$(CLI) config set directories.data shared/data
	$(CLI) config set directories.user shared
	$(CLI) config set directories.downloads shared/downloads
	$(CLI) config set network.connection_timeout 600s
	$(CLI) config add board_manager.additional_urls https://espressif.github.io/arduino-esp32/package_esp32_index.json
	$(CLI) core update-index
	$(CLI) core install "esp32:esp32@2.0.14"	# Compiles, uploads and the web page displays
#	$(CLI) core install "esp32:esp32"		# mbedtls_md5_xxx_ret routines  are not defined so PD_Stepper_Web_Server won't compile
	$(CLI) lib install "ESPAsyncWebServer"
	$(CLI) lib install "TMC2209"

uninit: ## `make unint` Remove all files/dirs created by running `make init`
	rm -rf arduino-cli.yaml build shared

lib.help: ## `make lib.help` Show lib help
	$(CLI) lib --help

lib.%: ## `make lib.<cmd> PKG=<PackageName>` Runs arduino-cli lib <cmd> "$(PKG)"
	$(CLI) lib $(@:lib.%=%) $(if $(PKG),"$(PKG)")

list l: ## `make l` List connected boards
	$(CLI) board list $(LFRMT) --discovery-timeout "$(DTO)"

core.%: ## `make lib.<cmd> PKG=<PackageName>` Runs arduino-cli lib <cmd> "$(PKG)"
	$(CLI) core $(@:core.%=%) $(if $(PKG),"$(PKG)")

# delete half-baked binary if compile fails
.DELETE_ON_ERROR: $(BUILD)/$(SKETCH).ino.bin

.PHONY: buildit
buildit:
	$(CLI) compile -b "$(FQBN)" --build-path "$(BUILD)" "./$(SKETCH)" $(OTHER_COMPILE_PARAMS); \

compile c: ## `make c S=<SketchDir>` Compile a sketch
	$(call check-sketch)
	@$(MAKE) buildit

compile_commands cc: ## `make cc S=<SketchDir>` Generate `build/compile_commands.json` using --only-compilation-database
	$(call check-sketch)
	@$(MAKE) buildit OTHER_COMPILE_PARAMS=--only-compilation-database

# Helper: resolve port (use PORT if set, else auto-detected), then run $(1)
define GET_PORT
	@P="$$( [ -n "$(PORT)" ] && printf "%s" "$(PORT)" \
	      || $(CLI) board list --discovery-timeout "$(DTO)" --json | jq -r '.detected_ports[0]?.port.address // empty' )"; \
	test -n "$$P" || { echo "Error: no serial port detected. Set PORT=/dev/ttyXXX"; exit 1; }; \
	$(1)
endef

upload u: compile ## `make u S=<SketchDir>` Compile if needed and Upload, (requires S or SKETCH, optional PORT)
	$(call GET_PORT,$(CLI) upload -p "$$P" -b "$(FQBN)" --input-dir "$(BUILD)")

monitor m: ## `make m {PORT=<port> BAUD=<baudrate>` Open serial monitor optional; PORT=auto-detected, BAUD=115200)
	$(call GET_PORT,$(CLI) monitor -p "$$P" -c baudrate="$(BAUD)")

clean cl: ## `make cl` Remove build artifacts
	rm -rf -- "$(BUILD)"
