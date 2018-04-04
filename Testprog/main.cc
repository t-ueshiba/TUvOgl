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

  // ロボットの右手第６軸に対する校正点の相対的位置を読み込み
    ifstream	in;
    openFile(in, configDirs, cameraName + ".hrp2");
    if (!in)
	throw runtime_error("Cannot open the robot pose parameter file!!");
    Vector4d	X6;
    in >> X6;
    cerr << "--- Target point ---\n" << X6 << endl;

  // 校正点の読み込み
    Array<Matrix44d>	poseList;
    in >> poseList;
    cerr << "--- Robot poses ---\n" << poseList;
    in.close();

  // 右手の把持中心を校正点に設定
    Vector3d	graspOffset = X6(0, 3);
    if (!hrp2.SetGraspCenter(false, graspOffset.data()))
	throw runtime_error("HRP2Client::SetGraspCenter() failed!!");
	
  // 初期姿勢へ
    hrp2.go_clothinit();

  // 各校正点について…
    for (int n = 0; n < poseList.size(); ++n)
    {
      // ロボットに次の把持中心（校正点）の姿勢を与える．
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

      // 軌道を生成する．
	hrp2.GenerateMotion();

      // ロボットが次の校正点に移動するのを待つ．
	hrp2.PlayMotion();

      // ロボット右手首第６軸の真の姿勢を求める。
	HRP2::TimedPose	Dw6;	// 第6軸座標系からワールド座標系への変換行列
	if (!hrp2.GetRealPose("RARM_JOINT6", Dw6))
	{
	    cerr << "HRP2Client::GetRealPose(): failed!" << endl;
	}
	else
	{
	  // 第６軸の真の姿勢が求まったら、それを把持中心の姿勢に変換する。
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
    
  // 初期姿勢へ
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
      // HRP2を初期化
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
