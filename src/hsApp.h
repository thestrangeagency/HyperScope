#pragma once

#include "ofMain.h"

class hsApp : public ofBaseApp
{
	public:

	void setup();
	void update();
	void draw();

	void keyPressed  (int key);
	void keyReleased(int key);
	void mouseMoved(int x, int y );
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
	void windowResized(int w, int h);
	void dragEvent(ofDragInfo dragInfo);
	void gotMessage(ofMessage msg);
	
	void audioOut(float * input, int bufferSize, int nChannels);
	
	
	ofSoundStream soundStream;

	float 	pan;
	int		sampleRate;
	bool 	bNoise;
	float 	volume;

	vector <float> voice1;
	vector <float> voice2;
	vector <float> voice3;
	
	float 	frequency;
	float 	targetFrequency;
	float 	phase1;
	float 	phase2;
	float 	phase3;
	float 	phaseAdder;
	
	int		numerator1;
	int		denominator1;
	int		numerator2;
	int		denominator2;
	
	float	rotator;

	int		widthi;
	float	widthf;
	
	ofEasyCam cam;
	ofMesh mesh;
};
