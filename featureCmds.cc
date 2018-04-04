/*
 *  $Id$
 */
#include <vector>
#include <string>
#include <boost/foreach.hpp>
#include "TU/v/vV4L2++.h"

namespace TU
{
namespace v
{
/************************************************************************
*  local data								*
************************************************************************/
static const size_t	NFEATURES = 30;
static CmdDef		featureCmds[2 + NFEATURES + 1];
    
/************************************************************************
*  global functions							*
************************************************************************/
CmdDef*
createFeatureCmds(const V4L2Camera& camera, size_t ncameras)
{
    static float		ranges[NFEATURES][3];
    static std::vector<MenuDef>	menus[NFEATURES];
    
    size_t	y	= 0;
    size_t	ncmds	= 0;
    size_t	nranges	= 0;
    size_t	nmenus	= 0;
    
    if (ncameras > 1)
    {
	static Array<CmdDef>		cameraChoiceCmds;
	static Array<std::string>	cameraChoiceTitles;
	
	cameraChoiceCmds  .resize(ncameras + 1);
	cameraChoiceTitles.resize(ncameras);
	
	for (size_t i = 0; i < ncameras; ++i)
	{
	    (cameraChoiceTitles[i] += "cam-") += ('0' + i);
	    
	    cameraChoiceCmds[i].type    = C_RadioButton;
	    cameraChoiceCmds[i].id	= i;
	    cameraChoiceCmds[i].val	= 0;
	    cameraChoiceCmds[i].title   = cameraChoiceTitles[i].c_str();
	    cameraChoiceCmds[i].gridx   = i;
	    cameraChoiceCmds[i].gridy	= 0;
	}
	cameraChoiceCmds[ncameras].type	= C_EndOfList;

	featureCmds[ncmds].type		= C_ChoiceFrame;
	featureCmds[ncmds].id		= V4L2CAMERA_CHOICE;
	featureCmds[ncmds].val		= 0;	// 最初のカメラを選択
	featureCmds[ncmds].prop		= cameraChoiceCmds.data();
	featureCmds[ncmds].gridx	= 0;
	featureCmds[ncmds].gridy	= y;
	++ncmds;
	
	featureCmds[ncmds].type		= C_ToggleButton;
	featureCmds[ncmds].id		= V4L2CAMERA_ALL;
	featureCmds[ncmds].val		= 1;	// 全カメラ同時操作モード
	featureCmds[ncmds].title	= "All";
	featureCmds[ncmds].gridx	= 1;
	featureCmds[ncmds].gridy	= y;
	++ncmds;

	++y;
    }

    BOOST_FOREACH (auto feature, camera.availableFeatures())
    {
	auto&	featureCmd = featureCmds[ncmds++];
	
	featureCmd.id	 = feature;
	featureCmd.val	 = camera.getValue(feature);
	featureCmd.title = camera.getName(feature).c_str();
	featureCmd.gridx = 0;
	featureCmd.gridy = y++;
	
	const auto	menuItems = camera.availableMenuItems(feature);

	if (menuItems.first == menuItems.second)
	{
	    int	min, max, step;
	    camera.getMinMaxStep(feature, min, max, step);

	    if (min == 0 && max == 1)
		featureCmd.type = C_ToggleButton;
	    else
	    {
		featureCmd.type = C_Slider;

		auto&	range = ranges[nranges++];
		range[0]	= min;
		range[1]	= max;
		range[2]	= step;
		featureCmd.prop = range;
	    }
	}
	else
	{
	    featureCmd.type = C_ChoiceMenuButton;

	    auto&	menu = menus[nmenus++];
	    
	    menu.resize(std::distance(menuItems.first, menuItems.second) + 1);
	    size_t	nitems = 0;
	    BOOST_FOREACH (const auto& menuItem, menuItems)
	    {
		menu[nitems].label   = menuItem.name.c_str();
		menu[nitems].id      = menuItem.index;
		menu[nitems].checked = (nitems == featureCmd.val);
		menu[nitems].submenu = noSub;
		++nitems;
	    }
	    menu[nitems].label = 0;

	    featureCmd.prop = menu.data();
	}
    }

    featureCmds[ncmds].type = C_EndOfList;

    return featureCmds;
}

void
refreshFeatureCmds(const V4L2Camera& camera, CmdPane& cmdPane)
{
    for (auto featureCmd = featureCmds;
	 featureCmd->type != C_EndOfList; ++featureCmd)
    {
	const auto	id = featureCmd->id;
	int		val;

	if (getFeature(camera, id, val))
	    cmdPane.setValue(id, val);
    }
}

}	// namespace v
}	// namespace TU
