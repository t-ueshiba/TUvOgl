/*
 *  $Id: main.cc,v 1.1.1.1 2002-07-25 02:14:15 ueshiba Exp $
 */
#include "TU/v/App.h"
#include "TU/v/CmdPane.h"
#include "TU/v/OglDC.h"

namespace TU
{
namespace v
{
/************************************************************************
*  menus and commands							*
************************************************************************/
static MenuDef FileMenu[] =
{
    {"New",  M_New,  false, noSub},
    {"Open", M_Open, false, noSub},
    {"-",    M_Line, false, noSub},
    {"Quit", M_Exit, false, noSub},
    EndOfMenu
};

static CmdDef MainMenu[] =
{
    {C_MenuButton, M_File, 0, "File", FileMenu, CA_None, 0, 0, 1, 1, 0},
    EndOfCmds
};

/************************************************************************
*  class MyCanvasPane							*
************************************************************************/
class MyCanvasPane : public CanvasPane
{
  public:
    MyCanvasPane(Window& parentWin)
	:CanvasPane(parentWin, 640, 480), _dc(*this)		{}

    OglDC&	dc()						{return _dc;}
    
    virtual void	repaintUnderlay(int x, int y, int w, int h)	;

  protected:
    virtual void	initializeGraphics()				;
    
  private:
    OglDC		_dc;
};

void
MyCanvasPane::repaintUnderlay(int, int, int, int)
{
    static const GLfloat	CX = -32.0, CY = 128.0, CZ = -16.0,
				LX =  64.0, LY =  16.0, LZ =  32.0;

    glClear(GL_COLOR_BUFFER_BIT);
    glBegin(GL_LINE_STRIP);
      glColor3f(1.0, 0.0, 0.0);
      glVertex3f(CX,	  CY,	   CZ);
      glVertex3f(CX + LX, CY,	   CZ);
      glColor3f(0.0, 1.0, 0.0);
      glVertex3f(CX,	  CY + LY, CZ);
      glColor3f(0.0, 0.0, 1.0);
      glVertex3f(CX,	  CY,	   CZ + LZ);
    glEnd();
    glFlush();
    _dc.swapBuffers();
}

void
MyCanvasPane::initializeGraphics()
{
    glClearColor(0.5, 0.5, 0.5, 1.0);
}

/************************************************************************
*  class MyCmdWindow							*
************************************************************************/
class MyCmdWindow : public CmdWindow
{
  public:
    MyCmdWindow(App&			parentApp,
		const char*		name,
		const XVisualInfo*	vinfo,
		Colormap::Mode	mode,
		u_int			resolution,
		u_int			underlayCmapDim,
		u_int			overlayCmapDim)		;

    virtual void	callback(CmdId, CmdVal)			;
    
  private:
    CmdPane		_menu;
    MyCanvasPane	_canvas;
};

MyCmdWindow::MyCmdWindow(App& parentApp, const char* name,
			 const XVisualInfo* vinfo, Colormap::Mode mode,
			 u_int resolution,
			 u_int underlayCmapDim, u_int overlayCmapDim)
    :CmdWindow(parentApp, name, vinfo, mode,
		  resolution, underlayCmapDim, overlayCmapDim),
     _menu(*this, MainMenu), _canvas(*this)
{
    _menu.place(0, 0, 1, 1);
    _canvas.place(0, 1, 1, 1);
    show();
}

void
MyCmdWindow::callback(CmdId id, CmdVal val)
{
    switch (id)
    {
      case M_Exit:
	app().exit();
	break;
    }
}
 
}
}
/************************************************************************
*  global functions							*
************************************************************************/
int
main(int argc, char* argv[])
{
    using namespace	TU;
    using namespace	std;

    v::App		vapp(argc, argv);

    int			attrs[] = {GLX_RGBA,
				   GLX_RED_SIZE,	1,
				   GLX_GREEN_SIZE,	1,
				   GLX_BLUE_SIZE,	1,
				   GLX_DEPTH_SIZE,	8,
				   GLX_DOUBLEBUFFER,
				   None};
    XVisualInfo*	vinfo = glXChooseVisual(vapp.colormap().display(),
						vapp.colormap().vinfo().screen,
						attrs);
    if (vinfo == 0)
    {
	cerr << "No appropriate visual!!" << endl;
	return 1;
    }
    
    v::MyCmdWindow	myWin0(vapp, "OpenGL test", vinfo,
			       v::Colormap::RGBColor, 16, 0, 0);
    vapp.run();

    cerr << "Loop exited!" << endl;

    return 0;
}
