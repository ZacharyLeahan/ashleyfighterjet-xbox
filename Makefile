XBE_TITLE = AshleyFighterJet
GEN_XISO = game.iso
SRCS = $(wildcard $(CURDIR)/src/*.c)
NXDK_SDL = y
DEBUG = y
CFLAGS += -O2 -Wall -Wextra
include $(NXDK_DIR)/Makefile
