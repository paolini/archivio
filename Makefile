VERSION= 1.1

DATA = $(shell date +"%d/%m/%Y")

OPT= -g -I/usr/include/mysql -DVERSION=\"$(VERSION)\" -DDATA=\"$(DATA)\"

TARDIR=$(HOME)/share/public_html/src/archivio

TARFILE=$(TARDIR)/archivio-$(shell date +"%Y%m%d").tgz

#CXX=g++-3.0

first: archivio

translate: translate.cpp
	$(CXX) $(OPT) -o $@ $<

cgi.o: cgi.cc cgi.h
	$(CXX) $(OPT) -c $<

outps.o: outps.cc outps.hh
	$(CXX) $(OPT) -c $<

archivio: archivio.o outps.o cgi.o 
	$(CXX) $(OPT) $^ -lmysqlclient  -o $@

archivio.o: archivio.cc mysql.hh
	$(CXX) $(OPT) -c $<

install: archivio
	install --mode=u+rwxs,go+rx $< /usr/lib/cgi-bin/archivio
#	cp $< /usr/lib/cgi-bin/archivio
#	chmod u+s /usr/lib/cgi-bin/archivio

clean:
	rm -f *~ *.o 

clear: clean
	rm -f archivio translate 

tar: clean
	strip archivio || echo non strippa
	strip translate || echo non strippa
	tar -C../.. -czvf $(TARFILE) archivio/src/
