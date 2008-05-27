/*
 *  $Id: main.cc,v 1.3 2008-05-27 11:38:25 ueshiba Exp $
 */
#include <iomanip>
#include <fstream>
#include <sstream>
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
enum	{m_X05 = 100, m_X10, m_X15, m_X20, m_Zoom};

static MenuDef ZoomMenu[] =
{
    {"x0.5", m_X05, false, noSub},
    {"x1",   m_X10, true,  noSub},
    {"x1.5", m_X15, false, noSub},
    {"x2",   m_X20, false, noSub},
    {NULL}
};

static MenuDef FileMenu[] =
{
    {"New",  M_New,  false, noSub},
    {"Open", M_Open, false, noSub},
    {"Zoom", m_Zoom, true,  &ZoomMenu[0]},
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
    MyCanvasPane(Window& parentWin, const Image<BGR>& image)
	:CanvasPane(parentWin, image.width(), image.height()),
	 _dc(*this), _image(image)				{}

    OglDC&		dc()					{return _dc;}
    
    virtual void	repaintUnderlay()			;
    virtual void	repaintOverlay()			;

  protected:
    virtual void	initializeGraphics()			;
    
  private:
    OglDC		_dc;
    Image<BGR>		_image;
};

void
MyCanvasPane::repaintUnderlay()
{
    static const GLfloat	CX = -32.0, CY = 128.0, CZ = -16.0,
				LX =  64.0, LY =  16.0, LZ =  32.0;

  //    glClear(GL_COLOR_BUFFER_BIT);
    _dc << _image << sync;
    
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
MyCanvasPane::repaintOverlay()
{
}

void
MyCanvasPane::initializeGraphics()
{
    _dc.setInternal(_image.width() / 2.0, _image.height() / 2.0, 400.0, 400.0,
		    1.0, 1000.0);
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
		const Image<BGR>&	image,
		const XVisualInfo*	vinfo,
		Colormap::Mode		mode,
		u_int			resolution,
		u_int			underlayCmapDim,
		u_int			overlayCmapDim)		;

    virtual void	callback(CmdId, CmdVal)			;
    
  private:
    CmdPane		_menu;
    MyCanvasPane	_canvas;
};

MyCmdWindow::MyCmdWindow(App& parentApp, const char* name,
			 const Image<BGR>& image,
			 const XVisualInfo* vinfo, Colormap::Mode mode,
			 u_int resolution,
			 u_int underlayCmapDim, u_int overlayCmapDim)
    :CmdWindow(parentApp, name, vinfo, mode,
	       resolution, underlayCmapDim, overlayCmapDim),
     _menu(*this, MainMenu), _canvas(*this, image)
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

      case m_X05:
	_canvas.dc() << x0_5 << repaintAll;
	break;
      case m_X10:
	_canvas.dc() << x1 << repaintAll;
	break;
      case m_X15:
	_canvas.dc() << x1_5 << repaintAll;
	break;
      case m_X20:
	_canvas.dc() << x2 << repaintAll;
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
    Image<BGR>		image;
    ifstream		in("../../../data/pbm/harumi.ppm", ios::in);
    image.restore(in);
    
    int			attrs[] = {GLX_RGBA,
				   GLX_RED_SIZE,	1,
				   GLX_GREEN_SIZE,	1,
				   GLX_BLUE_SIZE,	1,
				   GLX_DEPTH_SIZE,	1,
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
    
    v::MyCmdWindow	myWin0(vapp, "OpenGL test", image, vinfo,
			       v::Colormap::RGBColor, 16, 0, 3);
    vapp.run();

    cerr << "Loop exited!" << endl;

    return 0;
}
