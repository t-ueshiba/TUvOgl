/*
 *  $Id: OglDC.cc,v 1.6 2007-09-30 23:30:09 ueshiba Exp $
 */
#include "TU/v/OglDC.h"
#include <X11/Xmu/Converters.h>

namespace TU
{
namespace v
{
/************************************************************************
*  static functions							*
************************************************************************/
template <class T> static inline GLenum format()	{return GL_LUMINANCE;}
template <>	   static inline GLenum format<RGB>()	{return GL_RGB;}
template <>	   static inline GLenum format<RGBA>()	{return GL_RGBA;}
template <class T> static inline GLenum type()		{return
							     GL_UNSIGNED_BYTE;}
template <>	   static inline GLenum type<short>()	{return GL_SHORT;}
template <>	   static inline GLenum type<float>()	{return GL_FLOAT;}
    
/************************************************************************
*  class OglDC								*
************************************************************************/
OglDC::OglDC(CanvasPane& parentCanvasPane, u_int w, u_int h)
    :CanvasPaneDC(parentCanvasPane, w, h),
     CanvasPaneDC3(parentCanvasPane, w, h),
     _ctx(glXCreateContext(colormap().display(),
			   (XVisualInfo*)&(colormap().vinfo()), 0, True)),
     _nurbsRenderer(gluNewNurbsRenderer())
{
    XtVaSetValues(widget(), XtNbackingStore, NotUseful, NULL);
}

OglDC::~OglDC()
{
    if (_nurbsRenderer != 0)
	gluDeleteNurbsRenderer(_nurbsRenderer);
    glXDestroyContext(colormap().display(), _ctx);
}

DC&
OglDC::setSize(u_int width, u_int height, u_int mul, u_int div)
{
    CanvasPaneDC::setSize(width, height, mul, div);
    return setViewport();
}

DC3&
OglDC::setInternal(int u0, int v0, double ku, double kv,
		   double near, double far)
{
    makeCurrent();

    GLdouble	matrix[4][4];
    matrix[0][0] =  2.0 * ku / width();
    matrix[1][1] = -2.0 * kv / height();
    matrix[2][0] =  2.0 * u0 / width()  - 1.0;
    matrix[2][1] = -2.0 * v0 / height() + 1.0;
    matrix[2][3] =  1.0;
    if (far > near)
    {
	matrix[2][2] = (far + near) / (far - near);
	matrix[3][2] = -2.0 * far * near / (far - near);
    }
    else
    {
	matrix[2][2] =  1.0;
	matrix[3][2] = -2.0 * near;
    }
		   matrix[0][1] = matrix[0][2] = matrix[0][3] =
    matrix[1][0]	        = matrix[1][2] = matrix[1][3] =
    matrix[3][0] = matrix[3][1]		       = matrix[3][3] = 0.0;

    glMatrixMode(GL_PROJECTION);
      glLoadMatrixd(matrix[0]);
    glMatrixMode(GL_MODELVIEW);		// Default mode should be MODELVIEW.
    
    return *this;
}

DC3&
OglDC::setExternal(const Vector<double>& t, const Matrix<double>& Rt)
{
    makeCurrent();

  /* set rotation */
    GLdouble	matrix[4][4];
    for (int i = 0; i < 3; ++i)
    {
	for (int j = 0; j < 3; ++j)
	    matrix[i][j] = Rt[j][i];
	matrix[i][3] = matrix[3][i] = 0.0;
    }
    matrix[3][3] = 1.0;
    glLoadMatrixd(matrix[0]);
    
  /* set translation */
    glTranslated(-t[0], -t[1], -t[2]);
    
    return *this;
}

const DC3&
OglDC::getInternal(int& u0, int& v0, double& ku, double& kv,
		   double& near, double& far) const
{
    makeCurrent();

    GLdouble	matrix[4][4];
    glGetDoublev(GL_PROJECTION_MATRIX, matrix[0]);
    ku	 =	  matrix[0][0]  * width()  / 2.0;
    kv	 =       -matrix[1][1]  * height() / 2.0;
    u0	 = (1.0 + matrix[2][0]) * width()  / 2.0;
    v0	 = (1.0 - matrix[2][1]) * height() / 2.0;
    near =	 -matrix[3][2] / (1.0 + matrix[2][2]);
    far  = (matrix[2][2] != 1.0 ?
		  matrix[3][2] / (1.0 - matrix[2][2]) : 0.0);
    
    return *this;
}

const DC3&
OglDC::getExternal(Vector<double>& t, Matrix<double>& Rt) const
{
    makeCurrent();

    GLdouble	matrix[4][4];
    glGetDoublev(GL_MODELVIEW_MATRIX, matrix[0]);
    Rt.resize(3, 3);
    Rt[0][0] = matrix[0][0];
    Rt[0][1] = matrix[1][0];
    Rt[0][2] = matrix[2][0];
    Rt[1][0] = matrix[0][1];
    Rt[1][1] = matrix[1][1];
    Rt[1][2] = matrix[2][1];
    Rt[2][0] = matrix[0][2];
    Rt[2][1] = matrix[1][2];
    Rt[2][2] = matrix[2][2];
    t.resize(3);
    t[0] = -matrix[3][0];
    t[1] = -matrix[3][1];
    t[2] = -matrix[3][2];
    t *= Rt;
    
    return *this;
}

DC3&
OglDC::translate(double dist)
{
    makeCurrent();

  /* Since OpenGL does not support post-concatination, we have to do such a
     dirty thing. */
    GLdouble	matrix[4][4];
    glGetDoublev(GL_MODELVIEW_MATRIX, matrix[0]);	// Store the original.

    glLoadIdentity();
    GLdouble	dx = 0.0, dy = 0.0, dz = 0.0;
    switch (getAxis())
    {
      case DC3::X:
	dx = -dist;
	break;

      case DC3::Y:
	dy = -dist;
	break;

      case DC3::Z:
	dz = -dist;
	break;
    }
    glTranslated(dx, dy, dz);
    glMultMatrixd(matrix[0]);

    return DC3::translate(dist);
}

DC3&
OglDC::rotate(double angle)
{
    makeCurrent();

    GLdouble	matrix[4][4];
    glGetDoublev(GL_MODELVIEW_MATRIX, matrix[0]);	// Store the original.

    glLoadIdentity();
    glTranslated(0.0, 0.0, getDistance());
    GLdouble	nx = 0.0, ny = 0.0, nz = 0.0;
    switch (getAxis())
    {
      case DC3::X:
	nx = 1.0;
	break;

      case DC3::Y:
	ny = 1.0;
	break;

      default:
	nz = 1.0;
	break;
    }
    glRotated(-angle * 180.0 / M_PI, nx, ny, nz);
    glTranslated(0.0, 0.0, -getDistance());
    glMultMatrixd(matrix[0]);
    
    return *this;
}

void
OglDC::swapBuffers() const
{
    glXSwapBuffers(colormap().display(), drawable());
}

template <class T> Image<T>
OglDC::getImage() const
{
    Image<T>	image(deviceWidth(), deviceHeight());
    
    for (int v = 0; v < image.height(); ++v)
	glReadPixels(0, image.height() - 1 - v, image.width(), 1,
		     format<T>(), type<T>(), (T*)image[v]);
    return image;
}

void
OglDC::initializeGraphics()
{
    setViewport();
    CanvasPaneDC3::initializeGraphics();
}

OglDC&
OglDC::setViewport()
{
    makeCurrent();

    glViewport(0, widget().height() - deviceHeight(),
	       deviceWidth(), deviceHeight());
    return *this;
}

}
}
#if defined(__GNUG__) || defined(__INTEL_COMPILER)
#  include "TU/Image++.cc"
namespace TU
{
namespace v
{
template Image<RGB>	OglDC::getImage<RGB>()		const	;
template Image<RGBA>	OglDC::getImage<RGBA>()		const	;
template Image<u_char>	OglDC::getImage<u_char>()	const	;
template Image<short>	OglDC::getImage<short>()	const	;
template Image<float>	OglDC::getImage<float>()	const	;
}
}
#endif
