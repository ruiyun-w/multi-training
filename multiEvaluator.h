#pragma once
#include <vector>
#include <k4abt.h>
#include "Window3dWrapper.h"
#include <ctime>
#include <chrono>
#include <OpenXLSX.hpp>

using namespace std;
using namespace OpenXLSX;
using namespace std::chrono;

class multiEvaluator
{
public:
	// calculate score for each person
	float vectorEvaluator(k4abt_body_t body, XLWorksheet sheet);
private:
	// use 15 body vectors 
	int body_vector[5][4] = {
		{0,1,2,3},
		{3,5,6,7},
		{3,12,13,14},
		{0,18,19,20},
		{0,22,23,24}
	};
};

float multiEvaluator::vectorEvaluator(k4abt_body_t body, XLWorksheet sheet) 
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
	for (int i = 1 ; i < 5; i++) {
		for (int j = 1; j < 4; j++)
		{
			int index = body_vector[i][j];
			unitVector.xyz.x = unitVector.xyz.x + body.skeleton.joints[index].position.xyz.x - body.skeleton.joints[index-1].position.xyz.x;
			unitVector.xyz.y = unitVector.xyz.y + body.skeleton.joints[index].position.xyz.y - body.skeleton.joints[index-1].position.xyz.y;
			unitVector.xyz.z = unitVector.xyz.z + body.skeleton.joints[index].position.xyz.z - body.skeleton.joints[index-1].position.xyz.z;
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

