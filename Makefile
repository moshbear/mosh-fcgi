include Makefile.cxxopts

DIRS = include/ src/ examples/

all: $(DIRS)
	for dir in $^; do \
		cd $$dir; \
		make; \
		cd ..; \
	done

doc/*:
	doxygen

clean: $(DIRS)
	for dir in $^; do \
		cd $$dir; \
		make clean; \
		cd ..; \
	done
	
distclean: clean $(DIRS)
	for dir in $^; do \
		cd $$dir; \
		make distclean || echo "target $$dir lacks distclean" ; \
		cd ..; \
	done
	rm -r doc/
	
install: $(DIRS)
	for dir in $^; do \
		cd $$dir; \
		make install || echo "target $$dir lacks install"; \
		cd ..; \
	done
	TODO install_doc
	
.PHONY:  all clean distclean install doc


# Tell versions [3.59,3.63) of GNU make to not export all variables.
# Otherwise a system limit (for SysV at least) may be exceeded.
.NOEXPORT:
