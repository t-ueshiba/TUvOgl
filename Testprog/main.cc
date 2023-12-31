/*
 *  $Id: main.cc,v 1.7 2012-06-19 08:36:48 ueshiba Exp $
 */
#include <fstream>
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
    {"Save", M_Save, false, noSub},
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
    MyCanvasPane(Window& parentWin, double parallax)
	:CanvasPane(parentWin, 640, 480),
	 _dc(*this), _parallax(parallax)			{}

    OglDC&	dc()						{return _dc;}
    
    virtual void	repaintUnderlay()			;

  protected:
    virtual void	initializeGraphics()			;
    
  private:
    OglDC		_dc;
    const double	_parallax;
};

void
MyCanvasPane::repaintUnderlay()
{
    static const GLfloat	CX = -32.0, CY = 128.0, CZ = -16.0,
				LX =  64.0, LY =  16.0, LZ =  32.0;

    if (_parallax > 0)
    {
	_dc << v::axis(DC3::X);
	glPushMatrix();
	_dc << v::translate(-_parallax / 2.0);
	glDrawBuffer(GL_BACK_LEFT);
	glNewList(1, GL_COMPILE_AND_EXECUTE);
    }
    
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

    if (_parallax > 0)
    {
	glEndList();

	_dc << v::translate(_parallax);
	glDrawBuffer(GL_BACK_RIGHT);
	glClear(GL_COLOR_BUFFER_BIT);
	glCallList(1);
	glPopMatrix();
    }

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
		Colormap::Mode		mode,
		size_t			resolution,
		size_t			underlayCmapDim,
		size_t			overlayCmapDim,
		double			parallax)		;

    virtual void	callback(CmdId, CmdVal)			;
    
  private:
    CmdPane		_menu;
    MyCanvasPane	_canvas;
};

MyCmdWindow::MyCmdWindow(App& parentApp, const char* name,
			 const XVisualInfo* vinfo, Colormap::Mode mode,
			 size_t resolution,
			 size_t underlayCmapDim, size_t overlayCmapDim,
			 double parallax)
    :CmdWindow(parentApp, name, vinfo,
	       mode, resolution, underlayCmapDim, overlayCmapDim),
     _menu(*this, MainMenu), _canvas(*this, parallax)
{
    _menu.place(0, 0, 1, 1);
    _canvas.place(0, 1, 1, 1);
    show();

    _canvas.repaintUnderlay();
}

void
MyCmdWindow::callback(CmdId id, CmdVal val)
{
    switch (id)
    {
      case M_Exit:
	app().exit();
	break;

      case M_Save:
      {
	std::ofstream	out("dump.pbm");
	Image<RGB>	image = _canvas.dc().getImage<RGB>();
	image.save(out, ImageFormat::RGB_24);
      }
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
    const double	PARALLAX = 5.0;
    double		parallax = -1;
    extern char*	optarg;

    for (int c; (c = getopt(argc, argv, "s")) != -1; )
	switch (c)
	{
	  case 's':
	    parallax = PARALLAX;
	    break;
	}
    
    int			attrs[] = {GLX_RGBA,
				   GLX_RED_SIZE,	1,
				   GLX_GREEN_SIZE,	1,
				   GLX_BLUE_SIZE,	1,
				   GLX_DEPTH_SIZE,	1,
				   GLX_DOUBLEBUFFER,
				   GLX_STEREO,
				   None};
    if (parallax <= 0.0)
    {
	const int	nattrs = sizeof(attrs) / sizeof(attrs[0]);
	attrs[nattrs - 2] = None;
    }
    
    XVisualInfo*	vinfo = glXChooseVisual(vapp.colormap().display(),
						vapp.colormap().vinfo().screen,
						attrs);
    if (vinfo == 0)
    {
	cerr << "No appropriate visual!!" << endl;
	return 1;
    }
    
    v::MyCmdWindow	myWin(vapp, "OpenGL test", vinfo,
			      v::Colormap::RGBColor, 256, 0, 0, parallax);
    vapp.run();

    cerr << "Loop exited!" << endl;

    return 0;
}
