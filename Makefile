
all:
	$(MAKE) -C src

test:
	$(MAKE) test -C src

clean:
	$(MAKE) clean -C src

clean-contrib:
	$(MAKE) clean-contrib -C src	
	
