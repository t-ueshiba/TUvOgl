#ifndef __BODYINFO_H__
#define __BODYINFO_H__

#define R_HIP_Y		0
#define R_HIP_R		1
#define R_HIP_P		2
#define R_KNEE_P	3
#define R_ANKLE_P	4
#define R_ANKLE_R	5

#define L_HIP_Y  	6
#define L_HIP_R  	7
#define L_HIP_P  	8
#define L_KNEE_P 	9
#define L_ANKLE_P	10
#define L_ANKLE_R	11

#define CHEST_Y		12
#define CHEST_P		13

#define HEAD_Y		14
#define HEAD_P		15

#define R_SHOULDER_P	16
#define R_SHOULDER_R	17
#define R_SHOULDER_Y	18
#define R_ELBOW_P	19
#define R_WRIST_Y	20
#define R_WRIST_R	21
#define R_WRIST_P	22

#define R_THUMBCM_Y 	23
#define R_THUMBCM_P 	24
#define R_THUMBCM_R 	25
#define R_THUMBMP_P 	26
#define R_THUMBIP_P 	27
#define R_INDEXMP_R 	28
#define R_INDEXMP_P 	29
#define R_INDEXPIP_R 	30
#define R_INDEXDIP_R 	31
#define R_MIDDLEMP_R 	32
#define R_MIDDLEMP_P 	33
#define R_MIDDLEPIP_R 	34
#define R_MIDDLEDIP_R 	35
#define R_RINGMP_R 	36
#define R_RINGMP_P	37
#define R_RINGPIP_R	38
#define R_RINGDIP_R	39

#define L_SHOULDER_P    40
#define L_SHOULDER_R    41
#define L_SHOULDER_Y    42
#define L_ELBOW_P       43
#define L_WRIST_Y       44
#define L_WRIST_R       45
#define L_WRIST_P       46

#define L_THUMBCM_Y     47
#define L_THUMBCM_P     48
#define L_INDEXMP_R     49
#define L_INDEXMP_P     50
#define L_INDEXPIP_R    51
#define L_MIDDLEPIP_R	52

#define DOF		53

#define FORCE_SENSOR_NO	8
#define GYRO_SENSOR_NO	1
#define G_SENSOR_NO	1

enum {TEMP_RSP, TEMP_RSR, TEMP_RSY, TEMP_REP, TEMP_RLARM,
      TEMP_RWRP, TEMP_RWRR, TEMP_RWRY, TEMP_RUARM, TEMP_CHEST, 
      TEMP_WP, TEPM_WY, TEMP_CY, TEMP_LCY, TEMP_RULIMB, 
      TEMP_RAR, TEMP_RAP, TEMP_RKP, TEMP_RCP, TEMP_RCR, 
      TEMP_LULIMB, TEMP_LAR, TEMP_LAP, TEMP_LKP, TEMP_LCP, 
      TEMP_LCR, TEMP_WAIST, TEMP_LSP, TEMP_LSR, TEMP_LSY, 
      TEMP_LEP, TEMP_LLARM, TEMP_LWRP, TEMP_LWRR, TEMP_LWRY, 
      TEMP_LUARM, TEMP_NY, TEMP_NP, TEMP_HEAD,
      TEMP_CPU, TEMP_MB};

#define TEMP_SENSOR_NO	0

#define RLEG		0
#define	LLEG		1
#define RARM		2
#define	LARM		3
#define	TORSO		4
#define	HEAD		5
#define RHAND		6
#define	LHAND		7

#define	E_INVALID_ID	-3

#include "misc.h"

#define URL "HRP3/model/HRP3main.wrl"

#define MODEL_NAME	"HRP3"
#define WAIST_JOINT	"WAIST"
#define RLEG_END	"R_ANKLE_R"
#define LLEG_END	"L_ANKLE_R"
#define RARM_END	"R_WRIST_P"
#define LARM_END	"L_WRIST_P"
#define CHEST_JOINT	"CHEST_P"

#define SOLE_ROLL	(0)
#define SOLE_PITCH	(0)

#define ZMP_LIMIT	{{{0.04,-0.05},{0.085,-0.055}},\
			       {{0.05,-0.04},{0.085,-0.055}}}

#define FOOT_SIZE	{{0.13,0.1},{0.055, 0.075}}

#define ANKLE_HEIGHT	(0.111)
#define FSENSOR_OFFSET	(0.0)

#define FITTING_FZ_THD		(25) //[N]

#define TOUCH_THD		(25) //[N]

#define LEG_LINK_LEN1	(0.32) // [m]
#define LEG_LINK_LEN2	(0.32) // [m]
#define WAIST_HEIGHT	(LEG_LINK_LEN1+LEG_LINK_LEN2+ANKLE_HEIGHT)

#define HALF_SITTING_HIP_ANGLE	(deg2rad(-26.5))
#define HALF_SITTING_KNEE_ANGLE	(deg2rad(50))
#define HALF_SITTING_ANKLE_ANGLE	(-HALF_SITTING_HIP_ANGLE-HALF_SITTING_KNEE_ANGLE)
#define INITIAL_ZMP_REF_X	(0.0) // [m]
#define INITIAL_ZMP_REF_Z	(-1*(LEG_LINK_LEN1*cos(HALF_SITTING_HIP_ANGLE)+LEG_LINK_LEN2*cos(HALF_SITTING_ANKLE_ANGLE)+ANKLE_HEIGHT)) // [m]

#define TAU_THD		(3)
#define RWEIGHT		(530) // [N]


#define MIN_FZ		(25) // [N]

#define BUSH_K		(1.05e5)
#define BUSH_K_R	(620)
#define BUSH_K_T	((BUSH_K)*4)
#define BUSH_RAD	(0.0535)
#define BUSH_RATIO	(0.15)

#endif
