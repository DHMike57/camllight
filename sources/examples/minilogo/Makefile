# Makefile generated by imake - do not edit!
# $Xorg: imake.c,v 1.6 2001/02/09 02:03:15 xorgcvs Exp $

CAMLC=camlc
COMPFLAGS=-W
CAMLLIBR=camllibr
CAMLLEX=camllex
CAMLYACC=camlyacc
CPP=cpp -P -traditional -Dunix
CAMLDEP=camldep
RM=rm -f
TOP=./..
CONFIG=$(TOP)/config

all::

# (begin Imakefile)

all::logo

logo: crayon.zo langage.zo alex.zo asynt.zo logo.zo
	$(CAMLC) $(LINKFLAGS) -lunix -lgraph -lX11 -custom unix.zo graphics.zo crayon.zo langage.zo alex.zo asynt.zo logo.zo -o logo

clean::
	$(RM) logo

# (end Imakefile)

clean::
	$(RM) *.zi *.zix *.zo *~ #*#

beforedepend::

makefile: beforedepend
	imake -I$(CONFIG) -DTOPDIR=$(TOP)
	$(CAMLDEP) *.mli *.ml >> Makefile

makefiles:: makefile

.SUFFIXES: .mli .zi .ml .zo

.mli.zi:
	$(CAMLC) $(COMPFLAGS) -c $<

.ml.zo:
	$(CAMLC) $(COMPFLAGS) -c $<

# Generated dependencies below

asynt.zi: langage.zi alex.zi 
alex.zo: alex.zi 
asynt.zo: asynt.zi alex.zi langage.zi 
crayon.zo: crayon.zi 
langage.zo: langage.zi crayon.zi 
loadall.zo: logo.zo 
logo.zo: crayon.zi alex.zi langage.zi asynt.zi 
