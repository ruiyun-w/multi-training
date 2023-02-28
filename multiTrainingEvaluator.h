#pragma once
#include <vector>
#include <k4abt.h>
#include "Window3dWrapper.h"
#include <ctime>
#include <chrono>
#include <time.h>
#include <OpenXLSX.hpp>

using namespace std;
using namespace std::chrono;
using namespace OpenXLSX;
class multiEvaluator
{
public:
	// calculate angles
	float getAngle(k4a_float3_t A, k4a_float3_t B, k4a_float3_t C);
	// calculate score for each person
	float vectorEvaluator(k4abt_body_t body);
	// count jumps and return jump period time
	int jumpCounter(k4abt_body_t body, size_t body_num, XLWorksheet sheet);

	bool inJump[4] = { false };
	clock_t thisJumpTime[4] = { 0 };
	int jumpPeriod[4] = { 0 };

private:
	// use 15 body vectors 
	int body_vector[5][4] = {
		{0,1,2,3},
		{3,5,6,7},
		{3,12,13,14},
		{0,18,19,20},
		{0,22,23,24}
	};
	// jumpcount and period for 4 users
	int jumpCount[4] = { 0 };
	clock_t preJumpTime[4];
	int angleRow[4] = { 1 };
};

float multiEvaluator::getAngle(k4a_float3_t A, k4a_float3_t B, k4a_float3_t C)
{
	k4a_float3_t AbVector;
	k4a_float3_t BcVector;

	AbVector.xyz.x = B.xyz.x - A.xyz.x;
	AbVector.xyz.y = B.xyz.y - A.xyz.y;
	AbVector.xyz.z = B.xyz.z - A.xyz.z;

	BcVector.xyz.x = C.xyz.x - B.xyz.x;
	BcVector.xyz.y = C.xyz.y - B.xyz.y;
	BcVector.xyz.z = C.xyz.z - B.xyz.z;

	float AbNorm = (float)sqrt(AbVector.xyz.x * AbVector.xyz.x + AbVector.xyz.y * AbVector.xyz.y + AbVector.xyz.z * AbVector.xyz.z);
	float BcNorm = (float)sqrt(BcVector.xyz.x * BcVector.xyz.x + BcVector.xyz.y * BcVector.xyz.y + BcVector.xyz.z * BcVector.xyz.z);

	k4a_float3_t AbVectorNorm;
	k4a_float3_t BcVectorNorm;

	AbVectorNorm.xyz.x = AbVector.xyz.x / AbNorm;
	AbVectorNorm.xyz.y = AbVector.xyz.y / AbNorm;
	AbVectorNorm.xyz.z = AbVector.xyz.z / AbNorm;

	BcVectorNorm.xyz.x = BcVector.xyz.x / BcNorm;
	BcVectorNorm.xyz.y = BcVector.xyz.y / BcNorm;
	BcVectorNorm.xyz.z = BcVector.xyz.z / BcNorm;

	float result = AbVectorNorm.xyz.x * BcVectorNorm.xyz.x + AbVectorNorm.xyz.y * BcVectorNorm.xyz.y + AbVectorNorm.xyz.z * BcVectorNorm.xyz.z;

	result = 180.0f - (float)std::acos(result) * 180.0f / 3.1415926535897f;
	return result;
}

float multiEvaluator::vectorEvaluator(k4abt_body_t body)
{   //vector of each person, vj
	k4a_float3_t unitVector;
	k4a_float3_t unitVectorNorm;
	//vector of joints, vij
	k4a_float3_t jointVector;
	k4a_float3_t jointVectorNorm;
	// vij - vj
	k4a_float3_t dVector;
	//sum d(vij - vj)
	float distance = 0;

	//calculate vj
	unitVector.xyz.x = 0;
	unitVector.xyz.y = 0;
	unitVector.xyz.z = 0;
	for (int i = 1; i < 5; i++) {
		for (int j = 1; j < 4; j++)
		{
			int index = body_vector[i][j];
			unitVector.xyz.x = unitVector.xyz.x + body.skeleton.joints[index].position.xyz.x - body.skeleton.joints[index - 1].position.xyz.x;
			unitVector.xyz.y = unitVector.xyz.y + body.skeleton.joints[index].position.xyz.y - body.skeleton.joints[index - 1].position.xyz.y;
			unitVector.xyz.z = unitVector.xyz.z + body.skeleton.joints[index].position.xyz.z - body.skeleton.joints[index - 1].position.xyz.z;
		}
	}
	float unitNorm = (float)sqrt(unitVector.xyz.x * unitVector.xyz.x + unitVector.xyz.y * unitVector.xyz.y + unitVector.xyz.z * unitVector.xyz.z);
	unitVectorNorm.xyz.x = unitVector.xyz.x / unitNorm;
	unitVectorNorm.xyz.y = unitVector.xyz.y / unitNorm;
	unitVectorNorm.xyz.z = unitVector.xyz.z / unitNorm;

	//calculate sum d(vij - vj)
	for (int i = 1; i < 5; i++) {
		for (int j = 1; j < 4; j++)
		{   //vij
			int index = body_vector[i][j];
			jointVector.xyz.x = body.skeleton.joints[index].position.xyz.x - body.skeleton.joints[index - 1].position.xyz.x;
			jointVector.xyz.y = body.skeleton.joints[index].position.xyz.y - body.skeleton.joints[index - 1].position.xyz.y;
			jointVector.xyz.z = body.skeleton.joints[index].position.xyz.z - body.skeleton.joints[index - 1].position.xyz.z;
			float jointNorm = (float)sqrt(jointVector.xyz.x * jointVector.xyz.x + jointVector.xyz.y * jointVector.xyz.y + jointVector.xyz.z * jointVector.xyz.z);
			jointVectorNorm.xyz.x = jointVector.xyz.x / jointNorm;
			jointVectorNorm.xyz.y = jointVector.xyz.y / jointNorm;
			jointVectorNorm.xyz.z = jointVector.xyz.z / jointNorm;
			//vij - vj
			dVector.xyz.x = jointVectorNorm.xyz.x - unitVectorNorm.xyz.x;
			dVector.xyz.y = jointVectorNorm.xyz.y - unitVectorNorm.xyz.y;
			dVector.xyz.z = jointVectorNorm.xyz.z - unitVectorNorm.xyz.z;
			//sum d(vij - vj)
			distance = distance + (float)sqrt(dVector.xyz.x * dVector.xyz.x + dVector.xyz.y * dVector.xyz.y + dVector.xyz.z * dVector.xyz.z);
		}
	}

	return distance;
}

int multiEvaluator::jumpCounter(k4abt_body_t body, size_t body_num, XLWorksheet sheet) {
	k4abt_joint_t P_NECK = body.skeleton.joints[K4ABT_JOINT_NECK];
	k4abt_joint_t P_WRIST_RIGHT = body.skeleton.joints[K4ABT_JOINT_WRIST_RIGHT];
	k4abt_joint_t P_WRIST_LEFT = body.skeleton.joints[K4ABT_JOINT_WRIST_LEFT];
	k4abt_joint_t P_PELVIS = body.skeleton.joints[K4ABT_JOINT_PELVIS];

	float ANGLE_ARM_LEFT_PELVIS = getAngle(P_PELVIS.position, P_NECK.position, P_WRIST_LEFT.position);
	float ANGLE_ARM_RIGHT_PELVIS = getAngle(P_PELVIS.position, P_NECK.position, P_WRIST_RIGHT.position);
	//
	cout << body_num  << "ANGLE_ARM_RIGHT_PELVIS" << ANGLE_ARM_RIGHT_PELVIS << endl;
	cout << body_num << "ANGLE_ARM_LEFT_PELVIS" << ANGLE_ARM_LEFT_PELVIS << endl;

	milliseconds ms = duration_cast<milliseconds>(
		system_clock::now().time_since_epoch()
		);
	//write angles to excel 
	//sheet.cell(angleRow[body_num], body_num + 1).value() = ms.count();
	//sheet.cell(angleRow[body_num], body_num + 2).value() = ANGLE_ARM_LEFT_PELVIS;
	//sheet.cell(angleRow[body_num], body_num + 3).value() = ANGLE_ARM_RIGHT_PELVIS;
	//angleRow[body_num] = angleRow[body_num] + 1;

	// if arms up
	if ((ANGLE_ARM_LEFT_PELVIS > 100) && (ANGLE_ARM_RIGHT_PELVIS > 100) && (inJump[body_num] == false)) {
		//for first jump
		if (jumpCount[body_num] == 0) {
			preJumpTime[body_num] = clock();
		}
		//for other jump
		else {
			thisJumpTime[body_num] = clock();
			jumpPeriod[body_num] = thisJumpTime[body_num] - preJumpTime[body_num];
			preJumpTime[body_num] = thisJumpTime[body_num];
		}
		inJump[body_num] = true;
	}
	// if arms down
	else if ((ANGLE_ARM_LEFT_PELVIS < 30) && (ANGLE_ARM_RIGHT_PELVIS < 30) && (inJump[body_num] == true)) {
		inJump[body_num] = false;
		jumpCount[body_num] = jumpCount[body_num] + 1;
		cout << jumpCount[body_num] << endl;
	}

	return jumpPeriod[body_num];
}
