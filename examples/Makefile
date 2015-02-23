.PHONY: all clean

DIRS = echo/ filter/ raw-echo/ show-gnu/ timer/ upload/ utf8-helloworld/

all: $(DIRS)
	for dir in $^; do \
		cd $$dir; \
		make; \
		cd ..; \
	done

clean: $(DIRS)
	for dir in $^; do \
		cd $$dir; \
		make clean; \
		cd ..; \
	done
