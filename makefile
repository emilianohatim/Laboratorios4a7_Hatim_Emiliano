MODULES = Hal $(MUJU)/module/freertos
BOARD ?= edu-ciaa-nxp
VERBOSE=n
MUJU ?= muju
DOC_DIR = ./build/doc

doc:
	mkdir -p $(DOC_DIR)
	doxygen Doxyfile

include $(MUJU)/module/base/makefile