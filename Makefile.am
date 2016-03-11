ACLOCAL_AMFLAGS = -I m4

SUBDIRS = src

deb:
	$(MAKE) dist
	tar xf $(PACKAGE)-$(PACKAGE_VERSION).tar.gz
	cp -r debian $(PACKAGE)-$(PACKAGE_VERSION)/
	mv $(PACKAGE)-$(PACKAGE_VERSION).tar.gz $(PACKAGE)_$(PACKAGE_VERSION).orig.tar.gz
	cd $(PACKAGE)-$(PACKAGE_VERSION) && debuild -us -uc
