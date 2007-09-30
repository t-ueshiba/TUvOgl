#
#  $Id: Makefile,v 1.6 2007-09-30 23:30:09 ueshiba Exp $
#
#################################
#  User customizable macros	#
#################################
DEST		= $(BINDIR)
INCDIRS		= -I$(HOME)/include -I$(X11HOME)/include

PROGRAM		= ogltest
LIBS		= -lTUOgl++ -lTUv++ -lTUTools++ \
		  -lGLU -lGL -lXaw3d -lXt -lXext -lXmu -lX11

CPPFLAGS	=
CFLAGS		= -g
CCFLAGS		= -g
LDFLAGS		= -L$(LIBDIR) -L$(X11HOME)/lib
LINKER		= $(CCC)
ifeq ($(CCC), icc)
#  CCFLAGS	= -cxxlib-icc -O3 -parallel
  CCFLAGS	= -cxxlib-icc -g
  LDFLAGS      += -cxxlib-icc
  LINKER	= icpc
endif

#########################
#  Macros set by mkmf	#
#########################
SUFFIX		= .cc:sC
EXTHDRS		= /Users/ueshiba/include/TU/Array++.h \
		/Users/ueshiba/include/TU/Geometry++.h \
		/Users/ueshiba/include/TU/Image++.h \
		/Users/ueshiba/include/TU/List++.h \
		/Users/ueshiba/include/TU/Manip.h \
		/Users/ueshiba/include/TU/Vector++.h \
		/Users/ueshiba/include/TU/types.h \
		/Users/ueshiba/include/TU/v/App.h \
		/Users/ueshiba/include/TU/v/CanvasPane.h \
		/Users/ueshiba/include/TU/v/CanvasPaneDC.h \
		/Users/ueshiba/include/TU/v/CanvasPaneDC3.h \
		/Users/ueshiba/include/TU/v/CmdPane.h \
		/Users/ueshiba/include/TU/v/CmdWindow.h \
		/Users/ueshiba/include/TU/v/Colormap.h \
		/Users/ueshiba/include/TU/v/DC.h \
		/Users/ueshiba/include/TU/v/DC3.h \
		/Users/ueshiba/include/TU/v/Menu.h \
		/Users/ueshiba/include/TU/v/OglDC.h \
		/Users/ueshiba/include/TU/v/TUv++.h \
		/Users/ueshiba/include/TU/v/Widget-Xaw.h \
		/Users/ueshiba/include/TU/v/XDC.h
HDRS		=
SRCS		= main.cc
OBJS		= main.o

#########################
#  Macros used by RCS	#
#########################
REV		= $(shell echo $Revision: 1.6 $	|		\
		  sed 's/evision://'		|		\
		  awk -F"."					\
		  '{						\
		      for (count = 1; count < NF; count++)	\
			  printf("%d.", $$count);		\
		      printf("%d", $$count + 1);		\
		  }')

include $(PROJECT)/lib/p.mk
###
main.o: /Users/ueshiba/include/TU/v/App.h \
	/Users/ueshiba/include/TU/v/TUv++.h \
	/Users/ueshiba/include/TU/v/Colormap.h \
	/Users/ueshiba/include/TU/Image++.h \
	/Users/ueshiba/include/TU/Geometry++.h \
	/Users/ueshiba/include/TU/Vector++.h \
	/Users/ueshiba/include/TU/Array++.h /Users/ueshiba/include/TU/types.h \
	/Users/ueshiba/include/TU/List++.h \
	/Users/ueshiba/include/TU/v/Widget-Xaw.h \
	/Users/ueshiba/include/TU/v/CmdPane.h \
	/Users/ueshiba/include/TU/v/CmdWindow.h \
	/Users/ueshiba/include/TU/v/OglDC.h \
	/Users/ueshiba/include/TU/v/CanvasPaneDC3.h \
	/Users/ueshiba/include/TU/v/CanvasPaneDC.h \
	/Users/ueshiba/include/TU/v/XDC.h /Users/ueshiba/include/TU/v/DC.h \
	/Users/ueshiba/include/TU/Manip.h \
	/Users/ueshiba/include/TU/v/CanvasPane.h \
	/Users/ueshiba/include/TU/v/Menu.h /Users/ueshiba/include/TU/v/DC3.h
