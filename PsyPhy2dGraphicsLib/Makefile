INCLUDE	=	$(HOME)/include
LIB	=	$(HOME)/lib
BIN	=	$(HOME)/bin

CFLAGS	= 	-I . -I $(INCLUDE) -I /usr/local/include -I /usr/include -O -Xcpluscomm

LL	=	cc $(LFLAGS)
LFLAGS	=	-L . -L $(LIB)

AR	=	ar
ARFLAGS	=	rv


# Exported Items

DEFS	=	Views.h Layouts.h Displays.h Graphics.h Local.h LocalSGI.h SgiDisplay.h
LIBOBJS	=	Views.o Layouts.o Displays.o ArrayPlots.o SgiDisplay.o SgiMapDisplay.o FunctionPlots.o
OBJS	=	libViews.a
EXECS	=	

default: tst

install: $(DEFS) $(OBJS) $(EXECS) tst
	cp $(OBJS) $(LIB)
	cp $(DEFS) $(INCLUDE)
	touch install

clean:	
	touch clean.o clean.a tst core clean.ps
	rm *.o *.a *.ps *~
	rm $(EXECS) tst core

tst: 		tst.o libViews.a libs
	$(LL)	tst.o libViews.a \
		-lmatrix -luseful -lnrd -lXm -lXt -lX11 -lpsgl -lfm -lgl -lm -lc -o $@

libViews.a:	$(LIBOBJS)
	touch $@
	rm $@
	$(AR) $(ARFLAGS) $@ $(LIBOBJS)

Views.o: Views.c Views.h

Layouts.o: Layouts.c Layouts.h Views.h

ArrayPlots.o: ArrayPlots.c Views.h

Displays.o: Displays.c Displays.h Graphics.h

SgiDisplay.o: SgiDisplay.c SgiDisplay.h Displays.h Graphics.h

tst.o: tst.c Views.h

# check that libraries haven't changed.
libs: $(LIB)/libuseful.a $(LIB)/libmatrix.a $(LIB)/libnrd.a $(LIB)/libpsgl.a 
	touch libs
