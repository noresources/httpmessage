premake_options := 
make_options :=
 
ifdef cc
	premake_options += --cc=$(cc)
	premake__bin_options += --cc $(cc)
endif
ifdef os
	premake_options += --os=$(os)
endif
ifdef location
	premake_options += --location=$(location)
else
	location := build
endif
ifdef targetdir
	premake_options += --targetdir=$(targetdir)
endif
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
	
doc: doxygen
	@echo "Generating source documentation"
	@cd $(location) && doxygen httpmessage.doxyfile 1>/dev/null

$(location):
	@mkdir -p "$(location)"
	
doxygen: $(location)
	@echo Generation doxygen configuration file
	@premake5 --file=$(premake_filepath) $(premake_options) doxygen
	
preprocess: $(location)
	@echo Preprocess source files
	@premake5 --file=$(premake_filepath) $(premake_options) preprocess
	
premake: $(location) preprocess
	@echo Generate makefiles
	@premake5 --file=$(premake_filepath) $(premake_options) $(action)

clean:
	@echo Cleanup 
	@$(MAKE) -C "$(location)" $(make_options)  clean
	
help:
	@echo '------------------------------------------------------------'
	@echo 'httpmessage utility Makefile'
	@echo ' '
	@echo ' Actions'
	@echo ' '
	@echo ' * premake: Generate Makefiles using premake5'
	@echo '   Options '
	@echo '   * location: Build scripts location'
	@echo '   * targetdir: Location of build output'
	@echo '   * cc: Toolchain to use (gcc, clang)'
	@echo '   * os: Target platform (macosx, linux, ...)'
	@echo '   * action: (for premake target) Premake action to run'
	@echo '   For more generation options, use premake5 directly'
	@echo ' '
	@echo ' * doc: Generate documentation using Doxygen'
	@echo ' '
	@echo ' * astyle: Format code using Artistic Style (astyle)'
	@echo ' '
	
