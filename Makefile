#This makefile calls all other necessary makefiles

#set this to 1 if you want the model to support DLO output
export USE_DLO = 0

all: modules
clean: modules.clean

MODULES = platform pse model application
MODULES_CLEAN = $(patsubst %,%.clean,$(MODULES))

.PHONY: modules $(MODULES)
modules: $(MODULES)
$(MODULES):
	$(MAKE) -C $@ 

.PHONY: modules.clean $(MODULES_CLEAN)
modules.clean: $(MODULES_CLEAN)
$(MODULES_CLEAN):
	$(MAKE) -C $(@:.clean=) clean

