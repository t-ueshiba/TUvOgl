/*
 *  $Id$
 */
#include "TU/v/vV4L2++.h"
#include "TU/v/ModalDialog.h"

namespace TU
{
namespace v
{
/************************************************************************
*  class MyModalDialog							*
************************************************************************/
class MyModalDialog : public ModalDialog
{
  private:
    enum	{c_U0, c_V0, c_Width, c_Height, c_OK};
    
  public:
    MyModalDialog(Window& parentWindow, const V4L2Camera& camera)	;
    
    void		selectROI(size_t& u0, size_t& v0,
				  size_t& width, size_t& height)	;
    virtual void	callback(CmdId id, CmdVal val)			;

  private:
    CmdDef*		createROICmds(const V4L2Camera& camera)		;

  private:
    float	_ranges[4][3];
    CmdDef	_cmds[6];
};
    
MyModalDialog::MyModalDialog(Window& parentWindow, const V4L2Camera& camera)
    :ModalDialog(parentWindow, "ROI for V4L2 camera", createROICmds(camera))
{
}
    
void
MyModalDialog::selectROI(size_t& u0, size_t& v0, size_t& width, size_t& height)
{
    show();
    u0	   = pane().getValue(c_U0);
    v0	   = pane().getValue(c_V0);
    width  = pane().getValue(c_Width);
    height = pane().getValue(c_Height);
}

void
MyModalDialog::callback(CmdId id, CmdVal val)
{
    switch (id)
    {
      case c_OK:
	hide();
	break;
    }
}

CmdDef*
MyModalDialog::createROICmds(const V4L2Camera& camera)
{
    size_t	minU0, minV0, maxWidth, maxHeight;
    camera.getROILimits(minU0, minV0, maxWidth, maxHeight);
    size_t	u0, v0, width, height;
    camera.getROI(u0, v0, width, height);
    
  // Create commands for setting ROI.
    _ranges[0][0] = minU0;
    _ranges[0][1] = maxWidth - 1;
    _ranges[0][2] = 1;
    _cmds[0]	  = {C_Slider, c_U0,	 int(u0),     "    u0", _ranges[0],
		     CA_None, 0, 0, 1, 1, 0};

    _ranges[1][0] = minV0;
    _ranges[1][1] = maxHeight - 1;
    _ranges[1][2] = 1;
    _cmds[1]	  = {C_Slider, c_V0,	 int(v0),     "    v0", _ranges[1],
		     CA_None, 0, 1, 1, 1, 0};

    _ranges[2][0] = 0;
    _ranges[2][1] = maxWidth;
    _ranges[2][2] = 1;
    _cmds[2]	  = {C_Slider, c_Width,	 int(width),  " width", _ranges[2],
		     CA_None, 0, 2, 1, 1, 0};
    
    _ranges[3][0] = 0;
    _ranges[3][1] = maxHeight;
    _ranges[3][2] = 1;
    _cmds[3]	  = {C_Slider, c_Height, int(height), "height", _ranges[3],
		     CA_None, 0, 3, 1, 1, 0};

    _cmds[4]	  = {C_Button, c_OK, 0, "OK", noProp,
		     CA_None, 0, 4, 1, 1, 0};
    _cmds[5]	  = EndOfCmds;
	
    return _cmds;
}

/************************************************************************
*  global functions							*
************************************************************************/
bool
selectROI(V4L2Camera& camera, u_int id,
	  size_t& u0, size_t& v0, size_t& width, size_t& height, Window& window)
{
    if (id == V4L2Camera::UNKNOWN_PIXEL_FORMAT)
    {
	if (camera.getROI(u0, v0, width, height))
	{
	    MyModalDialog	modalDialog(window, camera);
	    modalDialog.selectROI(u0, v0, width, height);
	    camera.setROI(u0, v0, width, height);
	}

	return true;
    }
    else
	return false;
}

}
}
