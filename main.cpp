﻿#pragma comment(lib, "winmm")
#pragma comment(lib, "k4a.lib")
#pragma comment(lib, "k4abt.lib")
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <k4a/k4a.h>
#include <k4abt.h>
#include <iostream>
#include <math.h>
#include <mmdeviceapi.h>
#include <endpointvolume.h>
#include <audioclient.h>
#include <mmsystem.h>
#include <Mmsystem.h>
#include <string>
#include <BodyTrackingHelpers.h>
#include <Utilities.h>
#include <Window3dWrapper.h>
#include <multiTrainingEvaluator.h>
#include <tchar.h>
#include <thread>
#include <mutex>
#include <iostream>
#include <iomanip>
#include <OpenXLSX.hpp>
#include "MidiFile.h"
#include "Options.h"
#include <iomanip>
#include <cstdlib>
#include "RtMidi.h"
#include <Windows.h>


using namespace std;
using namespace OpenXLSX;
using namespace smf;

#define VERIFY(result, error)                                                                            \
    if(result != K4A_RESULT_SUCCEEDED)                                                                   \
    {                                                                                                    \
        printf("%s \n - (File: %s, Function: %s, Line: %d)\n", error, __FILE__, __FUNCTION__, __LINE__); \
        exit(1);                                                                                         \
    }       

bool s_isRunning = true;
int timer_start = 0;
int totalJumpPer = 0;
float aveJumpPer = 0;
int midiInterval = 0;
double tickDurationMilseconds;
int startTime;


int64_t ProcessKey(void* /*context*/, int key)
{
	// https://www.glfw.org/docs/latest/group__keys.html
	switch (key)
	{
		// Quit
	case GLFW_KEY_ESCAPE:
		s_isRunning = false;
		break;
	case GLFW_KEY_SPACE:
		s_isRunning = true;
		break;
	}
	return 1;
}

int64_t CloseCallback(void* /*context*/)
{
	s_isRunning = false;
	return 1;
}


void printAppUsage()
{
	printf("\n");
	printf(" Basic Usage:\n\n");
	printf(" 1. Make sure you place the camera parallel to the floor and there is only one person in the scene.\n");
	printf(" 2. Hit 'space' key to start training.\n");
	printf(" 3. Hit 'esc' key to stop app.\n");
	printf("\n");
	return;
}

void playMidiDrum(HMIDIOUT h) {
	const int SLEEP_INTERVAL = 10;
	midiOutShortMsg(h, 0x73c0);  // ���F���` �`�F��0x2a(42)
	while (true) {
		if (midiInterval > 0) {
			//int sleepedTime = 0;
			midiOutShortMsg(h, 0x7f4390);  // ���Ղ����� G5 0x43(67) 127 �`�F���l��0
			//int endTime = clock();
			Sleep(midiInterval);
			//cout << endTime - startTime << endl;
			//while (sleepedTime < midiInterval) {
			//	Sleep(SLEEP_INTERVAL);
			//	sleepedTime = sleepedTime + SLEEP_INTERVAL;
			//}
		}
	}

}

void playMidiFile(vector<MidiEvent*> noteOnEvent, RtMidiOut* midiout) {
	while (true) {
		if (tickDurationMilseconds > 0)
		{
			//play midi messages
			for (int event = 1; event < noteOnEvent.size(); event++) {
				midiout->sendMessage(noteOnEvent[event - 1]);
				int tickDuration = noteOnEvent[event]->tick - noteOnEvent[event - 1]->tick;
				if (tickDuration) {
					Sleep(int(tickDuration * tickDurationMilseconds));
					cout << tickDurationMilseconds << endl;
				}
				if (event == (noteOnEvent.size()))
					event = 1;
			}
		}
	}
}


void stopMidi(HMIDIOUT h) {
	int preInterval = 5000;
	while (true) {
		if (midiInterval - preInterval > 5000) {
			midiOutReset(h);
		}
		preInterval = midiInterval;
	}
}

int main()
{
	//open k4a device
	k4a_device_t device = NULL;
	VERIFY(k4a_device_open(0, &device), "Open K4A Device failed!");

	// Start camera. Make sure depth camera is enabled.
	k4a_device_configuration_t deviceConfig = K4A_DEVICE_CONFIG_INIT_DISABLE_ALL;
	deviceConfig.depth_mode = K4A_DEPTH_MODE_NFOV_UNBINNED;
	deviceConfig.color_resolution = K4A_COLOR_RESOLUTION_OFF;
	VERIFY(k4a_device_start_cameras(device, &deviceConfig), "Start K4A cameras failed!");

	// Get calibration information
	k4a_calibration_t sensor_calibration;
	VERIFY(k4a_device_get_calibration(device, deviceConfig.depth_mode, deviceConfig.color_resolution, &sensor_calibration),
		"Get depth camera calibration failed!");

	// Create Body Tracker
	k4abt_tracker_t tracker = NULL;
	k4abt_tracker_configuration_t tracker_config = K4ABT_TRACKER_CONFIG_DEFAULT;
	tracker_config.processing_mode = K4ABT_TRACKER_PROCESSING_MODE_GPU;
	VERIFY(k4abt_tracker_create(&sensor_calibration, tracker_config, &tracker), "Body tracker initialization failed!");
	int frame_count = 0;
	Window3dWrapper window3d;
	window3d.Create("3D Visualization", sensor_calibration);
	window3d.SetCloseCallback(CloseCallback);
	window3d.SetKeyCallback(ProcessKey);

	//Create training evaluator
	multiEvaluator evaluator;

	//Creat excel file to write angle data
	XLDocument doc;
	doc.create(".//multiTest.xlsx");
	auto wks = doc.workbook().worksheet("Sheet1");

	//Creat MIDI file player
	RtMidiOut* midiout = new RtMidiOut();
	MidiFile midifile("D:\\training\\MultiTraining\\MultiTraining\\media\\Midi_K525.mid");
	vector<MidiEvent*> noteOnEvent;

	//Creat midifile and print messages
	midifile.doTimeAnalysis();
	midifile.linkNotePairs();
	midifile.joinTracks();
	int tracks = midifile.getTrackCount();
	cout << "TPQ: " << midifile.getTicksPerQuarterNote() << endl;

	if (tracks > 1) cout << "TRACKS: " << tracks << endl;
	for (int track = 0; track < tracks; track++) {
		if (tracks > 1) cout << "\nTrack " << track << endl;
		cout << "Tick\tSeconds\tDur\tMessage" << endl;
		for (int event = 0; event < midifile[track].size(); event++) {
			cout << dec << midifile[track][event].tick;
			cout << '\t' << dec << midifile[track][event].seconds;
			cout << '\t';
			if (midifile[track][event].isNoteOn())
				cout << midifile[track][event].getDurationInSeconds();
			cout << '\t' << hex;
			for (int i = 0; i < midifile[track][event].size(); i++)
				cout << int(midifile[track][event][i]) << ' ';
			cout << endl;
		}
	}

	// Creat MidiEvent* vector
	for (int track = 0; track < tracks; track++) {
		for (int event = 0; event < midifile[track].size(); event++) {
			if (midifile[track][event].isNoteOn()) {
				noteOnEvent.push_back(&midifile[track][event]);
			}
		}
	}

	// Check available ports.
	unsigned int nPorts = midiout->getPortCount();
	if (nPorts == 0) {
		std::cout << "No ports available!\n";
	}

	// Open first available port.
	midiout->openPort(0);

	// Start play midi tile thread 
	thread playMidiFIleThread(playMidiFile, noteOnEvent, midiout);
	playMidiFIleThread.detach();
	//// Create MIDI player
	//HMIDIOUT h;
	//midiOutOpen(&h, MIDI_MAPPER, 0, 0, 0);
	//thread playMidiThread(playMidiDrum, h);
	//playMidiThread.detach();
	//thread stopMidiThread(stopMidi, h);
	//stopMidiThread.detach();

	//start camera
	while (s_isRunning)
	{

		k4a_capture_t sensor_capture;
		k4a_wait_result_t get_capture_result = k4a_device_get_capture(device, &sensor_capture, K4A_WAIT_INFINITE);
		if (get_capture_result == K4A_WAIT_RESULT_SUCCEEDED)
		{
			frame_count++;
			k4a_wait_result_t queue_capture_result = k4abt_tracker_enqueue_capture(tracker, sensor_capture, K4A_WAIT_INFINITE);
			k4a_capture_release(sensor_capture); // Remember to release the sensor capture once you finish using it
			if (queue_capture_result == K4A_WAIT_RESULT_TIMEOUT)
			{
				// It should never hit timeout when K4A_WAIT_INFINITE is set.
				printf("Error! Add capture to tracker process queue timeout!\n");
				break;
			}
			else if (queue_capture_result == K4A_WAIT_RESULT_FAILED)
			{
				printf("Error! Add capture to tracker process queue failed!\n");
				break;
			}

			k4abt_frame_t body_frame = NULL;
			k4a_wait_result_t pop_frame_result = k4abt_tracker_pop_result(tracker, &body_frame, K4A_WAIT_INFINITE);
			if (pop_frame_result == K4A_WAIT_RESULT_SUCCEEDED)
			{
				totalJumpPer = 0;
				// Successfully popped the body tracking result. Start processing
				k4a_capture_t original_capture = k4abt_frame_get_capture(body_frame);
				size_t num_bodies = k4abt_frame_get_num_bodies(body_frame);
				printf("%zu bodies are detected!\n", num_bodies);
				if (num_bodies > 0)
				{
					for (size_t i = 0; i < num_bodies; i++)
					{
						k4abt_body_t body;
						VERIFY(k4abt_frame_get_body_skeleton(body_frame, i, &body.skeleton), "Get skeleton from body frame failed!");
						body.id = k4abt_frame_get_body_id(body_frame, i);
						float score = evaluator.vectorEvaluator(body);
						int jumpPer = evaluator.jumpCounter(body, i, wks);
						switch (body.id)
						{
						case 1:
							printf("1st people jump period: %d\n ", jumpPer);
							break;
						case 2:
							printf("2nd people jump period: %d\n ", jumpPer);
							break;
						case 3:
							printf("3rd people jump period: %d\n ", jumpPer);
							break;
						case 4:
							printf("4st people jump period: %d\n", jumpPer);
							break;
						default:
							printf("Evaluate 4 people most!\n");

						}
						totalJumpPer = totalJumpPer + jumpPer;
					}
					aveJumpPer = totalJumpPer / num_bodies;
					printf("The average jump period %f\n", aveJumpPer);
					tickDurationMilseconds = aveJumpPer / 2 / 480;
				}

				// Visualize point cloud
				k4a_image_t depthImage = k4a_capture_get_depth_image(original_capture);
				window3d.UpdatePointClouds(depthImage);

				// Visualize the skeleton data
				window3d.CleanJointsAndBones();
				uint32_t numBodies = k4abt_frame_get_num_bodies(body_frame);
				for (uint32_t i = 0; i < numBodies; i++)
				{
					k4abt_body_t body;
					VERIFY(k4abt_frame_get_body_skeleton(body_frame, i, &body.skeleton), "Get skeleton from body frame failed!");
					body.id = k4abt_frame_get_body_id(body_frame, i);
					Color color = g_bodyColors[body.id % g_bodyColors.size()];
					window3d.AddBody(body, color);
				}
				//release the body frame once finish using it
				k4a_capture_release(original_capture);
				k4a_image_release(depthImage);
				k4abt_frame_release(body_frame);
			}

			else if (pop_frame_result == K4A_WAIT_RESULT_TIMEOUT)
			{
				//  It should never hit timeout when K4A_WAIT_INFINITE is set.
				printf("Error! Pop body frame result timeout!\n");
				break;
			}
			else
			{
				printf("Pop body frame result failed!\n");
				break;
			}
		}
		else if (get_capture_result == K4A_WAIT_RESULT_TIMEOUT)
		{
			// It should never hit time out when K4A_WAIT_INFINITE is set.
			printf("Error! Get depth frame time out!\n");
			break;
		}
		else
		{
			printf("Get depth capture returned error: %d\n", get_capture_result);
			break;
		}
		window3d.Render();
	}

	printf("Finished body tracking processing!\n");

	window3d.Delete();
	k4abt_tracker_shutdown(tracker);
	k4abt_tracker_destroy(tracker);
	k4a_device_stop_cameras(device);
	k4a_device_close(device);

	delete midiout;
	//midiOutReset(h);
	//midiOutClose(h);
	return 0;
}