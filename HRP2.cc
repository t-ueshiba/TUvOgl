/*
 *  For the communication between HRP2 control RTC's (currently ReachingRTC
 *  and SequencePlayerRTC) service provider and Vision program.
 *    by nkita 100808
 *
 *  Revised to support continuous monitoring hand poses and integrated into
 *  new library libTUHRP2++.
 *    by t.ueshiba 130301.
 *    
 *  $Id$
 */
#include "TU/HRP2++.h"
#include <cstdlib>
#include <cstdarg>
#include <boost/foreach.hpp>

#if defined(HRP3)
#  include "bodyinfo_HRP3.h"
#  include "bodyinfo_HRP2toHRP3.h"
#elif defined(HRP2YH)
#  include "bodyinfo_HRP2YH.h"
#elif defined(HRP2SH)
#  include "bodyinfo_HRP2SH.h"
#elif defined(HRP2KAI)
#  include "bodyinfo_HRP2KAI.h"
#  define R_WRIST_P	RARM_JOINT6
#  define L_WRIST_P	LARM_JOINT6
#  define RARM_HAND	"RARM_JOINT7"
#  define LARM_HAND	"LARM_JOINT7"
#else
#  include "bodyinfo_HRP2DOF7.h"
#endif

#if !defined(HALF_SITTING_WAIST_HEIGHT)
#  define HALF_SITTING_WAIST_HEIGHT	(LEG_LINK_LEN1*cos(HALF_SITTING_HIP_ANGLE)+LEG_LINK_LEN2*cos(HALF_SITTING_ANKLE_ANGLE)+ANKLE_HEIGHT)
#endif

#if defined(deg2rad)
#  undef deg2rad
#endif

namespace TU
{
/************************************************************************
*  static functions							*
************************************************************************/
static std::ostream&
printTime(std::ostream& out, u_int64_t localtime)
{
    u_int32_t	usec = localtime % 1000;
    u_int32_t	msec = (localtime / 1000) % 1000;
    u_int32_t	sec  = localtime / 1000000;
    return out << sec << '.' << msec << '.' << usec;
}

template <class ITER, class SEQ> static void
copyToSeq(ITER src, SEQ& seq, size_t n)
{
    seq.length(n);
    for (size_t i = 0; i < n; ++i)
	seq[i] = *src++;
}

template <class SEQ, class ITER> static void
copyFromSeq(SEQ& seq, ITER dst, size_t n)
{
    for (size_t i = 0; i < n; ++i)
	*dst++ = seq[i];
}

static inline HRP2::Time
usec(HRP2::Time sec, HRP2::Time nsec)
{
    return 1000000*sec + (nsec + 500)/1000;
}

template <class T> static inline T
deg2rad(T deg)
{
    return (deg * M_PI) / 180;
}
    
/************************************************************************
*  class HRP2								*
************************************************************************/
HRP2::HRP2(int argc, char* argv[], const char* linkName, u_int capacity)
    :_ior(0),
     _reaching(0), _motion(0), _seqplayer(0), _fk(0),
#ifdef HAVE_WALKGENERATOR
     _walkgenerator(0),
#endif
     _getRealPose(*this, linkName, capacity), _executeCommand(*this),
     _verbose(false)
{
    using namespace	std;
    
    if (!init(argc, argv))
	throw runtime_error("HRP2Client::init() failed!");
}

void
HRP2::setup(bool isLeftHand, bool isLaterMode)
{
    using namespace	std;

  // 拘束設定
    bool	constrained[] = {true, true, true, true, true, true};
    double	weights[]     = {10.0, 10.0, 10.0, 10.0, 10.0, 10.0};
    if (!SelectTaskDofs(isLeftHand, constrained, weights))
	throw runtime_error("HRP2Client::SelectTaskDofs() failed!!");

  // 使用する自由度を設定
    bool	usedDofs[] =
		{
		  // 6 + 6 + 2 + 2 = 16 DOFs
		    true, true, true, true, true, true,		// right leg
		    true, true, true, true, true, true,		// left leg
		    true, false,				// waist
		    false, false,				// neck
#if defined(HRP2SH) || defined(HRP2KAI)
		  // 7 + 1 + 7 + 1 + 5 + 5 = 26 DOFs
		    true, true, true, true, true, true,	true,	// right arm
		    false,					// right hand
		    false, false, false, false, false, false, false, // left arm
		    false,					// left hand
		    false, false, false, false, false,	// right hand joint
		    false, false, false, false, false,	// left hand joint
#else
		  // 7 + 1 + 7 + 1 = 16 DOFs
		    true, true, true, true, true, true, true,	// right arm
		    false,					// right hand
		    false, false, false, false, false, false, false, // left arm
		    false,					// left hand
#endif
		  // 6 DOFs
		    true, true, false, false, false, false	// base
		};
    if (!SelectUsedDofs(usedDofs))
	throw runtime_error("HRP2Client::SelectUsedDofs() failed!!");

  // ベースとなるリンクを設定
    if (!SelectBaseLink(RLEG_END))
	throw runtime_error("HRP2Client::SelectBaseLink() failed!!");

  // LaterモードまたはImmediateモードに設定
    if (!SelectExecutionMode(isLaterMode))
	throw runtime_error("HRP2Client::SelectExecutionMode() failed!!");

  // 反対側の手を強制的にdeselectし，所望の手をselectする．
    DeSelectArm(!isLeftHand);
    SelectArm(isLeftHand);

  // スレッドを起動する．
    _getRealPose.run();
    _executeCommand.run();
}

/*
 *  functions for ReachingService
 */
bool
HRP2::SelectArm(bool isLeft) const
{
    using namespace	OpenHRP;

    return isSuccess(_reaching->selectArm(isLeft ?
					  ReachingService::LEFT_ARM :
					  ReachingService::RIGHT_ARM),
		     " to select ", (isLeft ? "left" : "right"), " arm.", 0);
}

bool
HRP2::DeSelectArm(bool isLeft) const
{
    using namespace	OpenHRP;

    return isSuccess(_reaching->deselectArm(isLeft ?
					    ReachingService::LEFT_ARM :
					    ReachingService::RIGHT_ARM),
		     " to deselect ", (isLeft ? "left" : "right"), " arm.", 0);
}

bool
HRP2::isSelected(bool isLeft) const
{
    using namespace	OpenHRP;

    return isTrue(_reaching->isSelected(isLeft ? ReachingService::LEFT_ARM
					       : ReachingService::RIGHT_ARM),
		  " selected", (isLeft ? "left" : "right"), " arm.", 0);
}

bool
HRP2::isMaster(bool isLeft) const
{
    using namespace	OpenHRP;

    return isTrue(_reaching->isMaster(isLeft ? ReachingService::LEFT_ARM
					     : ReachingService::RIGHT_ARM),
		  " a master", (isLeft ? "left" : "right"), " arm", 0);
}

bool
HRP2::SetGraspCenter(bool isLeft, const double* pos) const
{
    using namespace	OpenHRP;

    ReachingService::dsequence dpos;
    copyToSeq(pos, dpos, 3);

    return isSuccess(_reaching->setGraspCenter((isLeft ?
						ReachingService::LEFT_ARM :
						ReachingService::RIGHT_ARM),
					       dpos),
		     " to set GraspCenter for ",
		     (isLeft ? "left" : "right"), " arm.", 0);
}

bool
HRP2::SelectUsedDofs(const bool* used) const
{
    using namespace	OpenHRP;

    ReachingService::bsequence	bused;
    copyToSeq(used, bused, DOF + 6);

    return isSuccess(_reaching->selectUsedDofs(bused),
		     " to select used DOFs.", 0);
}

bool
HRP2::SelectTaskDofs(bool isLeft,
		     const bool* constrained, const double* weights) const
{
    using namespace	OpenHRP;

    ReachingService::bsequence	bools;
    copyToSeq(constrained, bools, 6);

    ReachingService::dsequence	wei;
    copyToSeq(weights, wei, 6);

    return isSuccess(_reaching->selectTaskDofs((isLeft ?
						ReachingService::LEFT_ARM :
						ReachingService::RIGHT_ARM),
					       bools, wei),
		     " to select task DOF for ",
		     (isLeft ? "left" : "right"), " arm.", 0);
}

bool
HRP2::SelectExecutionMode(bool isLaterMode) const
{
    using namespace	OpenHRP;

    return isSuccess(_reaching->selectExecutionMode(
				  isLaterMode ? ReachingService::LATER
					      : ReachingService::IMMEDIATELY),
		     " to select ",
		     (isLaterMode ? "LATER" : "IMMEDIATELY"), " mode.", 0);
} 

bool
HRP2::SetTargetPose(bool isLeft, const double* pose, double duration) const
{
    using namespace	OpenHRP;

    ReachingService::dsequence	hmat;
    copyToSeq(pose, hmat, 16);

    return isSuccess(_reaching->setTargetPose((isLeft ?
					       ReachingService::LEFT_ARM :
					       ReachingService::RIGHT_ARM),
					      hmat, duration),
		     " to set target pose for ",
		     (isLeft ? "left" : "right"), " arm.", 0);
}

bool
HRP2::SetTargetArc(bool isLeft,
		   const double* rot, const double* cen,
		   const double* ax, double theta, double duration) const
{
    using namespace	OpenHRP;

    ReachingService::dsequence rotation;
    copyToSeq(rot, rotation, 9);
    ReachingService::dsequence center;
    copyToSeq(cen, center, 3);
    ReachingService::dsequence axis;
    copyToSeq(ax, axis, 3);

    return isSuccess(_reaching->setTargetArc((isLeft ?
					      ReachingService::LEFT_ARM :
					      ReachingService::RIGHT_ARM),
					     rotation, center, axis, theta,
					     duration),
		     " to set target arc for ", 
		     (isLeft ? "left" : "right"), " arm.", 0);
}

bool
HRP2::SetTargetVelocity(bool isLeft,
			const double* velocity, double duration) const
{
    using namespace	OpenHRP;

    ReachingService::dsequence	vel;
    copyToSeq(velocity, vel, 6);

    return isSuccess(_reaching->setTargetVelocity(
				    (isLeft ? ReachingService::LEFT_ARM
					    : ReachingService::RIGHT_ARM),
				    vel, duration),
		     " to set target velocity for ",
		     (isLeft ? "left" : "right"), " arm.", 0);
}

bool
HRP2::GetCurrentPose(bool isLeft, double* pose) const
{
    using namespace	OpenHRP;

    ReachingService::dsequence_var	hmat;
    bool ret = _reaching->getCurrentPose((isLeft ?
					  ReachingService::LEFT_ARM :
					  ReachingService::RIGHT_ARM),
					 hmat);
    if (ret)
	copyFromSeq(hmat, pose, 16);

    return isSuccess(ret, " to get current pose for ",
		     (isLeft ? "left" : "right"), " arm.", 0);
}

void
HRP2::GenerateMotion(bool blocked) const
{
    if (blocked)
    {
	_reaching->generateMotion();
	isSuccess(true, " to do generateMotion.", 0);
    }
    else
	_executeCommand(&HRP2::GenerateMotion);
} 

void
HRP2::PlayMotion(bool blocked) const
{
    if (blocked)
    {
	isSuccess(_reaching->playMotion(), " to do playMotion.", 0);
    }
    else
	_executeCommand(&HRP2::PlayMotion);
} 

bool
HRP2::isCompleted() const
{
    return _executeCommand.isCompleted();
}
    
bool
HRP2::GoRestPosture() const
{
    return isSuccess(_reaching->goRestPosture(), " to do goRestPosture.", 0);
} 

void
HRP2::getMotion()
{
    _motion = _reaching->getMotion();
}

u_int
HRP2::getMotionLength() const
{
    return _motion->length();
}

bool
HRP2::getPosture(u_int rank,
		 double*& q, double*& p, double*& rpy, double*& zmp)
{
    if (rank > _motion->length())
	return false;

    q   = _motion[rank].q.get_buffer();
    p   = _motion[rank].p.get_buffer();
    rpy = _motion[rank].rpy.get_buffer();
    zmp = _motion[rank].zmp.get_buffer();
    return true;
}

void
HRP2::ReverseRotation(bool isLeft) const
{
    using namespace	OpenHRP;

    _reaching->reverseRotation(isLeft ? ReachingService::LEFT_ARM
				      : ReachingService::RIGHT_ARM);
    isSuccess(true, " to do ReverseRotation for ",
	      (isLeft ? "left" : "right"), " arm.", 0);
} 

void
HRP2::EnableRelativePositionConstraint(bool on) const
{
    _reaching->enableRelativePositionConstraint(on);
    isTrue(on, " enabled relative position constraint.", 0);
} 

void
HRP2::EnableGazeConstraint(bool on) const
{
    _reaching->enableGazeConstraint(on);
    isTrue(on, " enabled gaze constraint.", 0);
} 

/*
 *  functions for ForwardKinematicsService
 */
bool
HRP2::SelectBaseLink(const char* linkname) const
{
    return isSuccess(_fk->selectBaseLink(linkname), " to select BaseLink.", 0);
} 

bool
HRP2::GetReferencePose(const char* linkname, TimedPose& D) const
{
    RTC::TimedDoubleSeq_var	hmat;
    bool			success = _fk->getReferencePose(linkname,
								hmat);
    if (success)
    {
	copyFromSeq(hmat->data, D.data(), D.nrow() * D.ncol());
	D.t = usec(hmat->tm.sec, hmat->tm.nsec);
    }

    return isSuccess(success, " to get reffrence pose.", 0);
}

bool
HRP2::GetRealPose(Time time, TimedPose& D) const
{
    return _getRealPose(time, D);
}

bool
HRP2::GetRealPose(const char* linkname, TimedPose& D) const
{
    RTC::TimedDoubleSeq_var	hmat;
    bool			success = _fk->getCurrentPose(linkname, hmat);
    if (success)
    {
	copyFromSeq(hmat->data, D.data(), D.nrow() * D.ncol());
	D.t = usec(hmat->tm.sec, hmat->tm.nsec);
    }

  //return isSuccess(success, " to get real pose.", 0);
    return success;
}

/*
 *  functions for SequencePlayerService
 */
void
HRP2::go_halfsitting()
{
    for (int i = 0; i < DOF; i++)
	_posture[DESIREDPOS][i] = _posture[HALFSIT][i];

    seqplay(HANDS);
}

void
HRP2::go_clothinit()
{
    for (int i = 0; i < DOF; i++)
	_posture[DESIREDPOS][i] = _posture[CLOTHINIT][i];

    seqplay(HANDS);
}

void
HRP2::go_pickupclothtableinitpos()
{
    go_leftpickupclothtableinitpos();
}

void
HRP2::go_leftpickupclothtableinitpos()
{
    _posture[DESIREDPOS][L_SHOULDER_P] = deg2rad(-105);
    _posture[DESIREDPOS][L_SHOULDER_R] = deg2rad(42);
    _posture[DESIREDPOS][L_SHOULDER_Y] = deg2rad(-72);
    _posture[DESIREDPOS][L_ELBOW_P]    = deg2rad(-106);
    _posture[DESIREDPOS][L_WRIST_Y]    = deg2rad(25);
    _posture[DESIREDPOS][L_WRIST_P]    = deg2rad(0);
    _posture[DESIREDPOS][L_HAND_P]     = deg2rad(45); // added by nkita 110214

    seqplay(HANDS);
}

void
HRP2::go_rightpickupclothtableinitpos()
{
    _posture[DESIREDPOS][R_SHOULDER_P] = deg2rad(-105);
    _posture[DESIREDPOS][R_SHOULDER_R] = deg2rad(-42);
    _posture[DESIREDPOS][R_SHOULDER_Y] = deg2rad(72);
    _posture[DESIREDPOS][R_ELBOW_P]    = deg2rad(-106);
    _posture[DESIREDPOS][R_WRIST_Y]    = deg2rad(-25);
    _posture[DESIREDPOS][R_WRIST_P]    = deg2rad(0);
    _posture[DESIREDPOS][R_HAND_P]     = deg2rad(45);

    seqplay(HANDS);
}

void
HRP2::go_leftpushhangclothpos()
{
    _posture[DESIREDPOS][L_SHOULDER_P] = deg2rad(-72.6);
    _posture[DESIREDPOS][L_SHOULDER_R] = deg2rad(95);
    _posture[DESIREDPOS][L_SHOULDER_Y] = deg2rad(-81);
    _posture[DESIREDPOS][L_ELBOW_P]    = deg2rad(-136.8);
    _posture[DESIREDPOS][L_WRIST_Y]    = deg2rad(82.8);
    _posture[DESIREDPOS][L_WRIST_P]    = deg2rad(0.0);
    _posture[DESIREDPOS][L_HAND_P]     = deg2rad(0.0);

    seqplay(HANDS);
}

void
HRP2::go_leftlowpushhangclothpos()
{
    _posture[DESIREDPOS][L_SHOULDER_P] = deg2rad(-35.8);
    _posture[DESIREDPOS][L_SHOULDER_R] = deg2rad(61.4);
    _posture[DESIREDPOS][L_SHOULDER_Y] = deg2rad(-80.9);
    _posture[DESIREDPOS][L_ELBOW_P]    = deg2rad(-137);
    _posture[DESIREDPOS][L_WRIST_Y]    = deg2rad(46.4);
    _posture[DESIREDPOS][L_WRIST_P]    = deg2rad(-7.4);
    _posture[DESIREDPOS][L_HAND_P]     = deg2rad(0);

    seqplay(HANDS);
}

void
HRP2::go_leftpushhangclothpos2()
{
    _posture[DESIREDPOS][L_SHOULDER_P] = deg2rad(-80);
    _posture[DESIREDPOS][L_SHOULDER_R] = deg2rad(60);
    _posture[DESIREDPOS][L_SHOULDER_Y] = deg2rad(-90);
    _posture[DESIREDPOS][L_ELBOW_P]    = deg2rad(-130);
    _posture[DESIREDPOS][L_WRIST_Y]    = deg2rad(0);
    _posture[DESIREDPOS][L_WRIST_P]    = deg2rad(0.0);
    _posture[DESIREDPOS][L_HAND_P]     = deg2rad(0.0);

    seqplay(HANDS);
}

void
HRP2::go_lefthangclothpos()
{
    _posture[DESIREDPOS][L_SHOULDER_P] = deg2rad(-145);
    _posture[DESIREDPOS][L_SHOULDER_R] = deg2rad(42.5);
    _posture[DESIREDPOS][L_SHOULDER_Y] = deg2rad(-90);
    _posture[DESIREDPOS][L_ELBOW_P]    = deg2rad(-99);
    _posture[DESIREDPOS][L_WRIST_Y]    = deg2rad(59);
    _posture[DESIREDPOS][L_WRIST_P]    = deg2rad(58.8);

    seqplay(HANDS);
}

void
HRP2::go_righthangclothpos()
{
    _posture[DESIREDPOS][R_SHOULDER_P] = deg2rad(-145);
    _posture[DESIREDPOS][R_SHOULDER_R] = deg2rad(-42.5);
    _posture[DESIREDPOS][R_SHOULDER_Y] = deg2rad(90);
    _posture[DESIREDPOS][R_ELBOW_P]    = deg2rad(-99);
    _posture[DESIREDPOS][R_WRIST_Y]    = deg2rad(-59);
    _posture[DESIREDPOS][R_WRIST_P]    = deg2rad(58.8);

    seqplay(HANDS);
}

void
HRP2::go_handopeningpos(bool isLeft, double angle) const
{
    using namespace	std;
    
    if (_verbose)
	cerr << "TU::HRP2: Begin to send HandOpening posture angles to SequencePlayerRTC." << endl;

    double time = 5.0;
    _seqplayer->setJointAngle((isLeft ? LARM_HAND : RARM_HAND),
#if defined(HRP2SH)
			      deg2rad(angle),
#else
			      deg2rad(isLeft ? angle : -angle),
#endif
			      time);
    while (!_seqplayer->isEmpty())
	;

    if (_verbose)
	cerr << "TU::HRP2: Finish to send HandOpening posture angles to SequencePlayerRTC." << endl;
}

void
HRP2::chest_rotate(int yaw, int pitch)
{
    _posture[DESIREDPOS][CHEST_Y] = deg2rad(yaw);
    _posture[DESIREDPOS][CHEST_P] = deg2rad(pitch);

    seqplay(EXCEPTHEAD);
}

void
HRP2::head_rotate(int yaw, int pitch)
{
    _posture[DESIREDPOS][HEAD_Y] = deg2rad(yaw);
    _posture[DESIREDPOS][HEAD_P] = deg2rad(pitch);

    seqplay(EXCEPTHEAD);
}

#ifdef HAVE_WALKGENERATOR
/*
 *  functions for WalkGeneratorService
 */
void
HRP2::walkTo(double x, double y, double theta) const
{
    if (_walkgenerator)
    {
	_walkgenerator->setTargetPosNoWait(x, y, deg2rad(theta));
	isSuccess(true, " to do walkTo.", 0);
    }
    return;
} 

void
HRP2::arcTo(double x, double y, double theta) const
{
    if (_walkgenerator)
    {
	_walkgenerator->setArcNoWait(x, y, deg2rad(theta));
	isSuccess(true, " to do arcTo.", 0);
    }
    return;
} 
#endif

/*
 *  private member functions
 */
bool
HRP2::init(int argc, char* argv[])
{
    using namespace	OpenHRP;
    using namespace	std;
    using		::optarg;
    using		::optind;
    
    const char*		nameServerHost = "localhost:2809";
    _verbose = false;

    ::opterr = 0;
    for (int c; (c = getopt(argc, argv, "N:V")) != EOF; )
	switch (c)
	{
	  case 'N':
	    nameServerHost = optarg;
	    break;
	  case 'V':
	    _verbose = true;
	    break;
	}
    ::opterr = 1;
    
  // ORBを取得
    CORBA::ORB_var	orb = CORBA::ORB_init(argc, argv);;

  // NamingServerを取得
    RTC::CorbaNaming*	naming;
    try
    {
	naming = new RTC::CorbaNaming(orb, nameServerHost);
    }
    catch (...)
    {
	cerr << "TU::HRP2: FAILED to get naming server." << endl;
	return false;
    }
    
  // ReachingServiceを取得
    isSuccess(_reaching = getService<ReachingService>("Reaching",
						      orb, naming),
	      " to get ReachingService.", 0);
    
    
  // SequencePlayerServiceを取得
    isSuccess(_seqplayer = getService<SequencePlayerService>("SequencePlayer",
							     orb, naming),
	      " to get SequencePlayerService.", 0);

  // ForwardKinematicsServiceを取得
    isSuccess(_fk = getService<ForwardKinematicsService>("ForwardKinematics",
							 orb, naming),
	      " to get ForwardKinematicsService.", 0);

#if HAVE_WALKGENERATOR
  // WalkGeneratorServiceを取得
    isSuccess(_walkgenerator = getService<WalkGeneratorService>("WalkGenerator",
								orb, naming),
	      " to get WalkGeneratorService.", 0);
#endif
    
    _posture[INITPOS].length(DOF);
    _posture[HALFSIT].length(DOF);
    _posture[CLOTHINIT].length(DOF);
    _posture[DESIREDPOS].length(DOF);
	
  // Set initial _posture
    for (int i = 0; i < DOF; i++)
	_posture[INITPOS][i] = _posture[HALFSIT][i] = 0;
    _posture[INITPOS][R_HAND_P] = deg2rad( 5);
    _posture[INITPOS][L_HAND_P] = deg2rad(-5);

  // Set half sitting posture
    _posture[HALFSIT][R_HIP_P]   = HALF_SITTING_HIP_ANGLE;
    _posture[HALFSIT][R_KNEE_P]  = HALF_SITTING_KNEE_ANGLE;
    _posture[HALFSIT][R_ANKLE_P] = HALF_SITTING_ANKLE_ANGLE;

    _posture[HALFSIT][L_HIP_P]   = HALF_SITTING_HIP_ANGLE;
    _posture[HALFSIT][L_KNEE_P]  = HALF_SITTING_KNEE_ANGLE;
    _posture[HALFSIT][L_ANKLE_P] = HALF_SITTING_ANKLE_ANGLE;

#if defined(HRP2SH)
    _posture[HALFSIT][R_SHOULDER_P] = deg2rad( 15);
    _posture[HALFSIT][R_SHOULDER_R] = deg2rad(-10);
    _posture[HALFSIT][R_ELBOW_P]    = deg2rad(-30);
    _posture[HALFSIT][R_HAND_P]	    = deg2rad( 20);

    _posture[HALFSIT][L_SHOULDER_P] = deg2rad( 15);
    _posture[HALFSIT][L_SHOULDER_R] = deg2rad( 10);
    _posture[HALFSIT][L_ELBOW_P]    = deg2rad(-30);
    _posture[HALFSIT][L_HAND_P]	    = deg2rad( 20);
#else
    _posture[HALFSIT][R_SHOULDER_P] = deg2rad( 15);
    _posture[HALFSIT][R_SHOULDER_R] = deg2rad(-10);
    _posture[HALFSIT][R_ELBOW_P]    = deg2rad(-30);
    _posture[HALFSIT][R_HAND_P]	    = deg2rad( 10);

    _posture[HALFSIT][L_SHOULDER_P] = deg2rad( 15);
    _posture[HALFSIT][L_SHOULDER_R] = deg2rad( 10);
    _posture[HALFSIT][L_ELBOW_P]    = deg2rad(-30);
    _posture[HALFSIT][L_HAND_P]	    = deg2rad(-10);
#endif

  // Set clothinit posture
    for (int i = 0; i < DOF; i++)
	_posture[CLOTHINIT][i] = _posture[DESIREDPOS][i]
			       = _posture[HALFSIT][i];
	
    _posture[CLOTHINIT][R_SHOULDER_P] = deg2rad(30);
    _posture[CLOTHINIT][R_SHOULDER_R] = 0.0;
  //_posture[CLOTHINIT][R_SHOULDER_R] = deg2rad(-10);
  //_posture[CLOTHINIT][R_ELBOW_P]    = deg2rad(-105);
    _posture[CLOTHINIT][R_ELBOW_P]    = deg2rad(-100);
    _posture[CLOTHINIT][R_WRIST_Y]    = 0.0;
    _posture[CLOTHINIT][R_WRIST_P]    = deg2rad(-15);

  // Set initial mask
    _mask[HANDS].length(DOF);
    _mask[LEFTHAND].length(DOF);
    _mask[RIGHTHAND].length(DOF);
    _mask[DESIREDMASK].length(DOF);
    _mask[EXCEPTHEAD].length(DOF);
	
    for (int i = 0; i < DOF; i++)
    {
	_mask[HANDS][i] = _mask[LEFTHAND][i]
			= _mask[RIGHTHAND][i] = _mask[DESIREDMASK][i] = 1;
	_mask[EXCEPTHEAD][i] = 0;
    }

    _mask[HANDS][L_HAND_P] = _mask[LEFTHAND][L_HAND_P] = 0;
    _mask[HANDS][R_HAND_P] = _mask[RIGHTHAND][R_HAND_P] = 0;
    _mask[EXCEPTHEAD][HEAD_Y] = _mask[EXCEPTHEAD][HEAD_P] = 1;

    return isSuccess(true, " to initialize HRP2.", 0);
}

template <class SERVICE> typename SERVICE::_ptr_type
HRP2::getService(const std::string& name,
		 CORBA::ORB_ptr orb, RTC::CorbaNaming* naming)
{
    using namespace	OpenHRP;
    using namespace	std;
    
  // RTCを取得
    RTC::CorbaConsumer<RTC::RTObject>	rtc;
    try
    {
	rtc.setObject(naming->resolve((name + "0.rtc").c_str()));
	rtc->_non_existent();
    }
    catch (...)
    {
	cerr << "TU::HRP2: FAILED to get RTC of " << name << '.' << endl;
	return 0;
    }

  // Serviceを取得
    if (!getServiceIOR(rtc, std::string((name + "Service").c_str())))
    {
	cerr << "TU::HRP2: FAILED to get IOR of " << name << "Service."
	     << endl;
	return 0;
    }

    return SERVICE::_narrow(orb->string_to_object(_ior));
}

bool
HRP2::getServiceIOR(RTC::CorbaConsumer<RTC::RTObject> rtc,
		    const std::string& serviceName)
{
    using namespace	std;
    
  // TargetRTCのポートリストを取得
    RTC::PortServiceList	ports = *(rtc->get_ports());
    if (ports.length() <= 0)
    {
	cerr << "TU::HRP2: FAILED to get PortServiceList of RTC." << endl;
	return false;
    }

    RTC::ComponentProfile*	cprof = rtc->get_component_profile();
    std::string			portname = std::string(cprof->instance_name)
					 + "." + serviceName;

    for (u_int i = 0; i < ports.length(); i++)
    {
	RTC::PortService_var	port = ports[i];
	RTC::PortProfile*	prof = port->get_port_profile();
	if (std::string(prof->name) == portname)
	{
	    RTC::ConnectorProfile	connProfile;
	    connProfile.name	     = "noname";
	    connProfile.connector_id = "";
	    connProfile.ports.length(1);
	    connProfile.ports[0]     = port;
	    connProfile.properties   = NULL;
	    port->connect(connProfile);

	    connProfile.properties[0].value >>= _ior;

	    port->disconnect(connProfile.connector_id);
	    
	    return true;
	}
    }

    return false;
}

bool
HRP2::isSuccess(bool success, ...) const
{
    using namespace	std;
    
    if (_verbose || !success)
    {
	va_list	args;
	va_start(args, success);
    
	cerr << "TU::HRP2:" << (success ? " Succeeded" : " FAILED");
	for (const char*s; (s = va_arg(args, const char*)) != 0; )
	    cerr << s;
	cerr << endl;

	va_end(args);
    }
    
    return success;
}

bool
HRP2::isTrue(bool ret, ...) const
{
    using namespace	std;
    
    if (_verbose)
    {
	va_list	args;
	va_start(args, ret);
    
	cerr << "TU::HRP2:" << (ret ? " " : " NOT");
	for (const char* s; (s = va_arg(args, const char*)) != 0; )
	    cerr << s;
	cerr << endl;

	va_end(args);
    }
    
    return ret;
}

void
HRP2::seqplay(mask_id id) const
{
    using namespace	OpenHRP;
    using namespace	std;

    if (_verbose)
	cerr << "TU::HRP2: Begin to send posture angles to SequencePlayerRTC."
	     << endl;
    
    double	time = 5.0;
    if (!_seqplayer->setJointAnglesWithMask(_posture[DESIREDPOS],
					    _mask[id], time))
	cerr << "TU::HRP2: setJointAnglesWithMask() FAILED." << endl;

    dSequence	waistpos;
    waistpos.length(3);
    waistpos[0] = waistpos[1] = 0.0;
    waistpos[2] = HALF_SITTING_WAIST_HEIGHT;
    if (!_seqplayer->setBasePos(waistpos, time))
	cerr << "TU::HRP2: setBasePos() FAILED." << endl;

    dSequence	waistrpy;
    waistrpy.length(3);
    waistrpy[0] = waistrpy[1] = waistrpy[2] = 0.0;
    if (!_seqplayer->setBaseRpy(waistrpy, time))
	cerr << "TU::HRP2: setBaseRpy() FAILED." << endl;

    dSequence	zmp;
    zmp.length(3);
    zmp[0] = zmp[1] = 0.0;
    zmp[2] = -HALF_SITTING_WAIST_HEIGHT;
    if (!_seqplayer->setZmp(zmp, time))
	cerr << "TU::HRP2: setZmp() FAILED." << endl;

    _seqplayer->waitInterpolation();
    
    if (_verbose)
	cerr << "TU::HRP2: Finish to send posture angles to SequencePlayerRTC."
	     << endl;
}

/************************************************************************
*  class HRP2::GetRealPoseThread					*
************************************************************************/
HRP2::GetRealPoseThread::GetRealPoseThread(const HRP2& hrp2,
					   const char* linkName,
					   u_int capacity)
    :_hrp2(hrp2), _linkName(linkName), _poses(capacity),
     _quit(false), _mutex(), _thread()
{
    pthread_mutex_init(&_mutex, NULL);
}

HRP2::GetRealPoseThread::~GetRealPoseThread()
{
    pthread_mutex_lock(&_mutex);
    _quit = true;			// 終了フラグを立てる
    pthread_mutex_unlock(&_mutex);

    pthread_join(_thread, NULL);	// 子スレッドの終了を待つ
    pthread_mutex_destroy(&_mutex);
}

void
HRP2::GetRealPoseThread::run()
{
    pthread_create(&_thread, NULL, threadProc, this);

    usleep(1000);
}
    
bool
HRP2::GetRealPoseThread::operator ()(Time time, TimedPose& D) const
{
  // 与えられた時刻よりも後のポーズが得られるまで待つ．
    TimedPose	after;		// timeよりも後の時刻で取得されたポーズ
    for (;;)			// を発見するまで待つ．
    {
#if 0
	using namespace	std;

	cerr << "Latest: ";
	printTime(cerr, _poses.back().t);
	cerr << endl;
#endif	
	pthread_mutex_lock(&_mutex);
	if (!_poses.empty() && (after = _poses.back()).t > time)
	    break;
	pthread_mutex_unlock(&_mutex);
    }

  // リングバッファを過去に遡り，与えられた時刻よりも前のポーズを探す．
    BOOST_REVERSE_FOREACH (const TimedPose& pose, _poses)
    {
	if (pose.t <= time)	// timeの直前のポーズならば．．．
	{
	  // poseとafterのうち，その時刻がtimeに近い方を返す．
	    if ((time - pose.t) < (after.t - time))
	    {			// timeがafterよりもposeの時刻に近ければ．．．
		D = pose;
	    }
	    else		// timeがposeよりもafterの時刻に近ければ．．．
	    {
		D = after;
	    }
	    pthread_mutex_unlock(&_mutex);

	    return true;	// timeを挟む２つのポーズを発見した
	}
	after = pose;
    }
    pthread_mutex_unlock(&_mutex);

    return false;		// timeの直前のポーズを発見できなかった
}

void
HRP2::GetRealPoseThread::timeSpan(Time& t0, Time& t1) const
{
    for (;;)
    {
	pthread_mutex_lock(&_mutex);
	if (!_poses.empty())
	{
	    t0 = _poses.front().t;
	    t1 = _poses.back().t;
	    pthread_mutex_unlock(&_mutex);
	    break;
	}
	pthread_mutex_unlock(&_mutex);
    }
}
    
void*
HRP2::GetRealPoseThread::mainLoop()
{
    for (;;)
    {
	pthread_mutex_lock(&_mutex);
	bool	quit = _quit;		// 命令を取得
	pthread_mutex_unlock(&_mutex);
	if (quit)			// 終了命令ならば...
	    break;			// 脱出

	TimedPose	D;
	if (_hrp2.GetRealPose(_linkName.c_str(), D))
	{					// ポーズ入力成功？
	    if (_poses.empty() || (D.t != _poses.back().t))
	    {
		pthread_mutex_lock(&_mutex);
		_poses.push_back(D);		// リングバッファに入れる
		pthread_mutex_unlock(&_mutex);
	    }
	}

	usleep(500);
    }

    return 0;
}
    
void*
HRP2::GetRealPoseThread::threadProc(void* thread)
{
    GetRealPoseThread*	th = static_cast<GetRealPoseThread*>(thread);

    return th->mainLoop();
}

/************************************************************************
*  class HRP2::ExecuteCommandThread					*
************************************************************************/
HRP2::ExecuteCommandThread::ExecuteCommandThread(const HRP2& hrp2)
    :_hrp2(hrp2), _commands(), _quit(false), _mutex(), _cond(), _thread()
{
    pthread_mutex_init(&_mutex, NULL);
    pthread_cond_init(&_cond, NULL);
}

HRP2::ExecuteCommandThread::~ExecuteCommandThread()
{
    pthread_mutex_lock(&_mutex);
    _quit = true;			// 終了命令をセット
    pthread_cond_signal(&_cond);	// 子スレッドに終了命令を送る
    pthread_mutex_unlock(&_mutex);

    pthread_join(_thread, NULL);	// 子スレッドの終了を待つ
    pthread_cond_destroy(&_cond);
    pthread_mutex_destroy(&_mutex);
}

void
HRP2::ExecuteCommandThread::run()
{
    pthread_create(&_thread, NULL, threadProc, this);

    usleep(1000);
}
    
void
HRP2::ExecuteCommandThread::operator ()(Command command) const
{
    pthread_mutex_lock(&_mutex);
    _commands.push(command);		// コマンドをキューに入れる
    pthread_cond_signal(&_cond);	// 子スレッドに実行命令を送る
    pthread_mutex_unlock(&_mutex);
}

void
HRP2::ExecuteCommandThread::wait() const
{
    pthread_mutex_lock(&_mutex);
    while (!_commands.empty())		// 全コマンドの実行が完了するまで
	pthread_cond_wait(&_cond, &_mutex);	// 待つ
    pthread_mutex_unlock(&_mutex);
}

bool
HRP2::ExecuteCommandThread::isCompleted() const
{
    pthread_mutex_lock(&_mutex);
    bool	empty = _commands.empty();	// キューが空？
    pthread_mutex_unlock(&_mutex);

    return empty;			// キューが空ならば実行完了
}
    
void*
HRP2::ExecuteCommandThread::mainLoop()
{
    pthread_mutex_lock(&_mutex);
    for (;;)
    {
	pthread_cond_wait(&_cond, &_mutex);	// 親からの命令を待つ
	if (_quit)				// スレッド終了命令ならば...
	    break;				// ループを脱出
	else if (!_commands.empty())		// 実行命令ならば...
	{
	    Command	command = _commands.front();	// 最古のコマンド
	    pthread_mutex_unlock(&_mutex);
	    (_hrp2.*command)(true);		// 最古のコマンドを実行
	    pthread_mutex_lock(&_mutex);
	    _commands.pop();			// 最古のコマンドを捨てる
	    pthread_cond_signal(&_cond);	// 親に実行が完了したことを通知
	}
    }
    pthread_mutex_unlock(&_mutex);

    return 0;
}

void*
HRP2::ExecuteCommandThread::threadProc(void* thread)
{
    ExecuteCommandThread* th = static_cast<ExecuteCommandThread*>(thread);

    return th->mainLoop();
}

}
