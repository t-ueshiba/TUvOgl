/*
 *  $Id: main.cc,v 1.8 2009-08-07 05:09:12 ueshiba Exp $
 */
#include <cstdlib>
#include <stdexcept>
#include "TU/Manip.h"
#include "TU/HRP2++.h"

#define DEFAULT_CONFIG_DIRS	".:/usr/local/etc/cameras"
#define DEFAULT_CAMERA_NAME	"IEEE1394Camera"

namespace TU
{
/************************************************************************
*  static functions							*
************************************************************************/
static std::string
openFile(std::ifstream& in, const std::string& dirs, const std::string& name)
{
    using namespace		std;

    string::const_iterator	p = dirs.begin();
    do
    {
	string::const_iterator	q = find(p, dirs.end(), ':');
	string			fullName = string(p, q) + '/' + name;
	in.open(fullName.c_str());
	if (in)
	    return fullName;
	p = q;
    } while (p++ != dirs.end());

    throw runtime_error("Cannot open input file \"" + name +
			"\" in \"" + dirs + "\"!!");
    return string();
}
    
static void
doJob(HRP2& hrp2, const std::string& configDirs, const std::string& cameraName)
{
    using namespace	std;

  // $B%m%\%C%H$N1&<jBh#6<4$KBP$9$k9;@5E@$NAjBPE*0LCV$rFI$_9~$_(B
    ifstream	in;
    openFile(in, configDirs, cameraName + ".hrp2");
    if (!in)
	throw runtime_error("Cannot open the robot pose parameter file!!");
    Vector4d	X6;
    in >> X6;
    cerr << "--- Target point ---\n" << X6 << endl;

  // $B9;@5E@$NFI$_9~$_(B
    Array<Matrix44d>	poseList;
    in >> poseList;
    cerr << "--- Robot poses ---\n" << poseList;
    in.close();

  // $B1&<j$NGD;}Cf?4$r9;@5E@$K@_Dj(B
    Vector3d	graspOffset = X6(0, 3);
    if (!hrp2.SetGraspCenter(false, graspOffset.data()))
	throw runtime_error("HRP2Client::SetGraspCenter() failed!!");
	
  // $B=i4|;Q@*$X(B
    hrp2.go_clothinit();

  // $B3F9;@5E@$K$D$$$F!D(B
    for (int n = 0; n < poseList.size(); ++n)
    {
      // $B%m%\%C%H$K<!$NGD;}Cf?4!J9;@5E@!K$N;Q@*$rM?$($k!%(B
	cout << "--- Target-" << n << '/' << poseList.size() - 1 << " ---\n"
	     << poseList[n];
	double	duration = 10.0;

	if (n == 0 || n == 2 || n == 7 || n == 8)
	    duration = 15.0;
	
	if (!hrp2.SetTargetPose(false, poseList[n].data(), duration))
	{
	    cerr << "HRP2Client::SetTargetPose(): failed!" << endl;
	    continue;
	}

      // $B50F;$r@8@.$9$k!%(B
	hrp2.GenerateMotion();

      // $B%m%\%C%H$,<!$N9;@5E@$K0\F0$9$k$N$rBT$D!%(B
	hrp2.PlayMotion();

      // $B%m%\%C%H1&<j<sBh#6<4$N??$N;Q@*$r5a$a$k!#(B
	HRP2::TimedPose	Dw6;	// $BBh(B6$B<4:BI87O$+$i%o!<%k%I:BI87O$X$NJQ499TNs(B
	if (!hrp2.GetRealPose("RARM_JOINT6", Dw6))
	{
	    cerr << "HRP2Client::GetRealPose(): failed!" << endl;
	}
	else
	{
	  // $BBh#6<4$N??$N;Q@*$,5a$^$C$?$i!"$=$l$rGD;}Cf?4$N;Q@*$KJQ49$9$k!#(B
	    Matrix44d	D6g = diag<4>(1.0);
	    D6g[0][3] = X6[0];
	    D6g[1][3] = X6[1];
	    D6g[2][3] = X6[2];
	    Matrix44d	Dwg = Dw6 * D6g;
	    cout << "--- Real-" << n << '/' << poseList.size() - 1 << " ---\n"
		 << Dwg;
	}

	cerr << "Hit RETURN key >> ";
	cin >> skipl;
    }
    
  // $B=i4|;Q@*$X(B
    hrp2.go_clothinit();
}
    
}
/************************************************************************
*  global functions							*
************************************************************************/
int
main(int argc, char* argv[])
{
    using namespace	std;
    using namespace	TU;


  // Main job.
    try
    {
      // HRP2$B$r=i4|2=(B
	HRP2		hrp2(argc, argv);
	hrp2.setup(false, true);

      // Parse command options.
	string		configDirs = DEFAULT_CONFIG_DIRS;
	string		cameraName = DEFAULT_CAMERA_NAME;
	extern char*	optarg;
	for (int c; (c = getopt(argc, argv, "d:c:")) != EOF; )
	    switch (c)
	    {
	      case 'd':
		configDirs = optarg;
		break;
	      case 'c':
		cameraName = optarg;
		break;
	    }
    
	doJob(hrp2, configDirs, cameraName);
    }
    catch (exception& err)
    {
	cerr << err.what() << endl;
	return 1;
    }

    return 0;
}
