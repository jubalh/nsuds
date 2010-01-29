SUBDIRS = src

ourhelpdir = "${datarootdir}/doc/${PACKAGE_NAME}-${VERSION}/"
dist_ourhelp_DATA = helpfiles/main
 
# Debug Mode
#  - Don't forget to run 'make clean' before switching
debug:
	$(MAKE) all "CFLAGS=-g -DDEBUG"


