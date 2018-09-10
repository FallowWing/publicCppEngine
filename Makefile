# This must always be assets or buildAssetLib will fail a grep
ASSET_PATH := assets
BIN_ASSET_PATH := bin/assets
SOURCE_AUDIO_PATH := ../sourceAssets/audio

# This is always ./bin for windows build
BIN_PATH := bin

CPP_TOOLS := ..
CPP_TOOLS_WIN := ..
ANDROID_DIR := ~/android_dev

ifeq ($(shell echo $$USERNAME), MintPaw)
	CROSSBRIDGE_PATH = /d/_tools/_sdks/crossbridge
	JAVA_MAKE_PATH := /d/_tools/_sdks/jdk8u144_x64
	AIR_MAKE_PATH := /d/_tools/_sdks/air
	CUR_MAKE_PATH := /e/Dropbox/cpp/engine
	ANDROID_SDK_PATH := /d/_tools/_sdks/android-sdk
	ANT_PATH := /d/_tools/_sdks/ant
	EMSCRIPTEN_DIR := /d/_tools/_sdks/emsdk-portable/emscripten/1.37.21
	PYTHON3 := /c/Users/MintPaw/AppData/Local/Programs/Python/Python36/python.exe
endif

ifeq ($(shell echo $$USERNAME), Amaranta)
	CROSSBRIDGE_PATH = /d/tools/sdks/crossbridge
	JAVA_MAKE_PATH := /d/tools/sdks/jre8_x64
	AIR_MAKE_PATH := /d/tools/sdks/air
	CUR_MAKE_PATH := /c/Users/Amaranta/Dropbox/cpp/engine
	ANDROID_SDK_PATH := /d/tools/sdks/android-sdk
	ANT_PATH := /d/tools/sdks/ant
	EMSCRIPTEN_DIR := /d/_tools/_sdks/emsdk-portable/emscripten/1.37.21
	PYTHON3 := /c/Users/MintPaw/AppData/Local/Programs/Python/Python36/python.exe
endif

GAME_WIDTH=1280
GAME_HEIGHT=720

all:
	$(MAKE) defaultAll

include $(CPP_TOOLS)/engine/buildSystem/Makefile.common
