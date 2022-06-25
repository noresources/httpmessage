premake_options := 
make_options :=

location = $(CURDIR)/build
targetdir = $(CURDIR)/dist
 
ifdef cc
	premake_options += --cc=$(cc)
	premake__bin_options += --cc $(cc)
endif

ifdef os
	premake_options += --os=$(os)
endif

premake_options += --location=$(location)
premake_options += --targetdir=$(targetdir)

ifndef action
	action = gmake2
endif

premake_filepath=scripts/premake/premake5.lua

.PHONY: all astyle clean doc doxygen help  premake preprocess

all: 
	@$(MAKE) -C "$(location)" $(make_options) 

astyle:
	@echo Apply code style
	@astyle --options=scripts/astyle/c.style src/apps/*.c src/httpmessage/*.c include/httpmessage/*.h tests/*.c tests/*.h
	
doc: doxygen $(targetdir)
	@echo "Generating source documentation"
	@doxygen scripts/doxygen/httpmessage.doxyfile

doxygen: $(location)
	@echo Generation doxygen configuration file
	@premake5 --file=$(premake_filepath) $(premake_options) doxygen
	
$(location):
	@mkdir -p "$(location)"

preprocess: $(location)
	@echo Preprocess source files
	@premake5 --file=$(premake_filepath) $(premake_options) preprocess
	
premake: $(location) preprocess
	@echo Generate makefiles
	@premake5 --file=$(premake_filepath) $(premake_options) $(action)

$(targetdir):
	@mkdir -p "$(targetdir)"
	
clean:
	@echo Cleanup 
	@$(MAKE) -C "$(location)" $(make_options)  clean
	
help:
	@echo "@@ $(CURDIR) @@"
	@echo '------------------------------------------------------------'
	@echo 'httpmessage utility Makefile'
	@echo ' '
	@echo ' Targets'
	@echo ' '
	@echo ' * premake: Generate Makefiles using premake5'
	@echo ' * doc: Generate documentation using Doxygen'
	@echo ' * astyle: Format code using Artistic Style (astyle)'
	@echo ' '
	@echo ' Options '
	@echo ' '
	@echo ' * location: Build scripts location'
	@echo ' * targetdir: Targets output base directory'
	@echo ' * cc: Toolchain to use (gcc, clang)'
	@echo ' * os: Target platform (macosx, linux, ...)'
	@echo ' * action: (for premake target) Premake action to run'
	@echo ' '
	
