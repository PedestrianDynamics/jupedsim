.PHONY: release debug clean clean-release clean-debug install

DIRS:= bin bin/release bin/debug

all: $(DIRS) release


$(DIRS):
	mkdir $@


release:
	( cd bin/release && cmake -DCMAKE_BUILD_TYPE=release ../.. && $(MAKE) --no-print-directory )
#	ctags -R  --language-force=c++ *.*
#	ctags -eR  --language-force=c++ *.*

debug:
	( cd bin/debug && cmake -DCMAKE_BUILD_TYPE=debug ../.. && $(MAKE) --no-print-directory && ctest)
#	ctags -R  --language-force=c++ *.*
#	ctags -eR  --language-force=c++ *.*

clean: clean-release clean-debug

clean-release:
	( cd bin/release && $(MAKE) --no-print-directory clean )

clean-debug:
	( cd bin/debug && $(MAKE) --no-print-directory clean )


#release:
#	( cd build/release && cmake -DCMAKE_BUILD_TYPE=release ../.. && $(MAKE) --no-print-directory && make  --no-print-directory install)
#	ctags -R  --language-force=c++ *.*
#	ctags -eR  --language-force=c++ *.*
