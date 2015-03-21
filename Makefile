
all:
	$(MAKE) -C src

lib:
	$(MAKE) lib -C src

clean:
	$(MAKE) clean -C src

clean-contrib:
	$(MAKE) clean-contrib -C src	

