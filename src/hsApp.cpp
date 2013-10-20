#include "hsApp.h"

//--------------------------------------------------------------
void hsApp::setup()
{
	//ofBackground(34, 34, 34);
	ofSetVerticalSync(true);
	
	widthi = 512;
	widthf = 512.f;
	
	mesh.setMode(OF_PRIMITIVE_POINTS);
	
	for(int y = 0; y < widthi; y++ )
	{
		for(int x = 0; x < widthi; x++ )
		{
			ofColor color(12,150,255,255);
			mesh.addColor(color);
			ofVec3f pos(0, 0, 0);
			mesh.addVertex(pos);
		}
	}
	
	ofEnableDepthTest();
	glEnable(GL_POINT_SMOOTH); // use circular points instead of square points
	glPointSize(3); // make the points bigger
	
	// 2 output channels,
	// 0 input channels
	// 22050 samples per second
	// 512 samples per buffer
	// 4 num buffers (latency)
	
	int bufferSize		= 512;
	sampleRate 			= 44100;
	phase1				= 0;
	phase2				= 0;
	phase3				= 0;
	phaseAdder 			= 0.0f;
	volume				= 0.1f;
	bNoise 				= false;
	
	voice1.assign(bufferSize, 0.0);
	voice2.assign(bufferSize, 0.0);
	voice3.assign(bufferSize, 0.0);
	
	frequency = targetFrequency = 0;
	numerator1 = numerator2 = 2;
	denominator1 = denominator2 = 3;
	rotator = 0;
	
	//soundStream.listDevices();
	
	//if you want to set the device id to be different than the default
	//soundStream.setDeviceID(1); 	//note some devices are input only and some are output only
	
	soundStream.setup(this, 2, 0, sampleRate, bufferSize, 4);
	
	ofSetFrameRate(60);
}


//--------------------------------------------------------------
void hsApp::update()
{
}

//--------------------------------------------------------------
void hsApp::draw()
{
	ofBackgroundGradient(ofColor::gray, ofColor::black, OF_GRADIENT_CIRCULAR);
	
	vector<ofVec3f>& verts = mesh.getVertices();
	vector<ofFloatColor>& color = mesh.getColors();
	
	for(unsigned int i = 0; i < verts.size(); i++)
	{
		int j = ofMap(i, 0, verts.size(), 0, widthf);
		
		verts[i].x = voice1[j] * widthf;
		verts[i].y = voice2[j] * widthf;
		verts[i].z = voice3[j] * widthf;
		
		//color[i].r = 128.f + voice1[j] * 127.f;
		//color[i].g = 128.f + voice2[j] * 127.f;
	}
	
	// even points can overlap with each other, let's avoid that
	cam.begin();
	//ofScale(2, -2, 2); // flip the y axis and zoom in a bit
	ofRotateY(rotator);
	rotator += 1.f;
	//ofTranslate(ofGetHeight() / 2, ofGetHeight() / 2);
	mesh.draw();
	cam.end();
	
	
	//
	
	
	ofSetColor(225);
	ofDrawBitmapString("AUDIO OUTPUT EXAMPLE", 32, 32);
	ofDrawBitmapString("press 's' to unpause the audio\npress 'e' to pause the audio", 31, 92);
	
	ofNoFill();
	
	// draw the left channel:
	ofPushStyle();
	ofPushMatrix();
	ofTranslate(32, 150, 0);
	
	ofSetColor(225);
	ofDrawBitmapString("Left Channel", 4, 18);
	
	ofSetLineWidth(1);
	ofRect(0, 0, 900, 200);
	
	ofSetColor(245, 58, 135);
	ofSetLineWidth(3);
	
	ofBeginShape();
	for (unsigned int i = 0; i < voice1.size(); i++)
	{
		float x =  ofMap(i, 0, voice1.size(), 0, 900, true);
		ofVertex(x, 100 -voice1[i]*180.0f);
	}
	ofEndShape(false);
	
	ofPopMatrix();
	ofPopStyle();
	
	// draw the right channel:
	ofPushStyle();
	ofPushMatrix();
	ofTranslate(32, 350, 0);
	
	ofSetColor(225);
	ofDrawBitmapString("Right Channel", 4, 18);
	
	ofSetLineWidth(1);
	ofRect(0, 0, 900, 200);
	
	ofSetColor(245, 58, 135);
	ofSetLineWidth(3);
	
	ofBeginShape();
	for (unsigned int i = 0; i < voice2.size(); i++)
	{
		float x =  ofMap(i, 0, voice2.size(), 0, 900, true);
		ofVertex(x, 100 -voice2[i]*180.0f);
	}
	ofEndShape(false);
	
	ofPopMatrix();
	ofPopStyle();
	
	
	ofSetColor(225);
	string reportString = "volume: ("+ofToString(volume, 2)+") modify with -/+ keys\npan: ("+ofToString(pan, 2)+") modify with mouse x\nsynthesis: ";
	if( !bNoise )
	{
		reportString += "sine wave (" + ofToString(frequency, 2) + " > " + ofToString(targetFrequency, 2) + "hz) modify with mouse y";
	}
	else
	{
		reportString += "noise";
	}
	reportString += "\nratios = " + ofToString(numerator1) + ":" + ofToString(denominator1) + ", " + ofToString(numerator2) + ":" + ofToString(denominator2);
	ofDrawBitmapString(reportString, 32, 579);
	
}


//--------------------------------------------------------------
void hsApp::keyPressed  (int key)
{
	if (key == '-' || key == '_' )
	{
		volume -= 0.05;
		volume = MAX(volume, 0);
	}
	else if (key == '+' || key == '=' )
	{
		volume += 0.05;
		volume = MIN(volume, 1);
	}
	
	if( key == 's' )
	{
		soundStream.start();
	}
	
	if( key == 'e' )
	{
		soundStream.stop();
	}
	
	if( key == 'q' )
	{
		numerator1--;
		if( numerator1 < 1 ) numerator1 = 1;
	}
	
	if( key == 'w' )
	{
		numerator1++;
	}
	
	if( key == 'a' )
	{
		denominator1--;
		if( denominator1 < 1 ) denominator1 = 1;
	}
	
	if( key == 's' )
	{
		denominator1++;
	}
	
	if( key == 'i' )
	{
		numerator2--;
		if( numerator2 < 1 ) numerator2 = 1;
	}
	
	if( key == 'o' )
	{
		numerator2++;
	}
	
	if( key == 'k' )
	{
		denominator2--;
		if( denominator2 < 1 ) denominator2 = 1;
	}
	
	if( key == 'l' )
	{
		denominator2++;
	}
	
}

//--------------------------------------------------------------
void hsApp::keyReleased  (int key)
{
}

//--------------------------------------------------------------
void hsApp::mouseMoved(int x, int y )
{
	int width = ofGetWidth();
	pan = (float)x / (float)width;
	float height = (float)ofGetHeight();
	targetFrequency = 27.f * (height-y);
}

//--------------------------------------------------------------
void hsApp::mouseDragged(int x, int y, int button)
{
	int width = ofGetWidth();
	pan = (float)x / (float)width;
}

//--------------------------------------------------------------
void hsApp::mousePressed(int x, int y, int button)
{
	//bNoise = true;
}


//--------------------------------------------------------------
void hsApp::mouseReleased(int x, int y, int button)
{
	//bNoise = false;
}

//--------------------------------------------------------------
void hsApp::windowResized(int w, int h)
{
}

//--------------------------------------------------------------
void hsApp::audioOut(float * output, int bufferSize, int nChannels)
{
	while (phase1 > TWO_PI)
	{
		phase1 -= TWO_PI;
	}
	while (phase2 > TWO_PI)
	{
		phase2 -= TWO_PI;
	}
	while (phase3 > TWO_PI)
	{
		phase3 -= TWO_PI;
	}
	
	if ( bNoise == true)
	{
		// ---------------------- noise --------------
		for (int i = 0; i < bufferSize; i++)
		{
			voice1[i] = ofRandom(0, 1) * volume;
			voice2[i] = ofRandom(0, 1) * volume;
			voice3[i] = ofRandom(0, 1) * volume;
			
			output[i*nChannels] = output[i*nChannels + 1] = voice1[i] + voice2[i] + voice3[i];
		}
	}
	else
	{
		frequency = 0.98f * frequency + 0.02f * targetFrequency;
		phaseAdder = (frequency / (float) sampleRate) * TWO_PI;
		
		for (int i = 0; i < bufferSize; i++)
		{
			phase1 += phaseAdder;
			phase2 += phaseAdder*float(numerator1)/float(denominator1);
			phase3 += (phaseAdder*float(numerator1)/float(denominator1))*float(numerator2)/float(denominator2);
			
			float sample1 = sin(phase1);
			float sample2 = sin(phase2);
			float sample3 = sin(phase3);
			
			voice1[i] = sample1 * volume;
			voice2[i] = sample2 * volume;
			voice3[i] = sample3 * volume;
			
			output[i*nChannels] = output[i*nChannels + 1] = voice1[i] + voice2[i] + voice3[i];
		}
	}
	
}

//--------------------------------------------------------------
void hsApp::gotMessage(ofMessage msg)
{
}

//--------------------------------------------------------------
void hsApp::dragEvent(ofDragInfo dragInfo)
{
}
