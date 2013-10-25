#include "hsApp.h"

//--------------------------------------------------------------
void hsApp::setup()
{
	//ofBackground(34, 34, 34);
	ofSetVerticalSync(true);
	ofSetFrameRate(30);
	
	widthi = 512;
	widthf = widthi;
	
	mesh.setMode(OF_PRIMITIVE_POINTS);
	
	for(int y = 0; y < widthi; y++ )
	{
		for(int x = 0; x < widthi; x++ )
		{
			ofFloatColor color(x/widthf,y/widthf,1.f,0.5f);
			mesh.addColor(color);
			ofVec3f pos(x-widthf/2.f, y-widthf/2.f, 0);
			mesh.addVertex(pos);
		}
	}
	
	ofEnableDepthTest();
	glEnable(GL_POINT_SMOOTH); // use circular points instead of square points
	
	// 2 output channels,
	// 0 input channels
	// 44100 samples per second
	// 512 samples per buffer
	// 4 num buffers (latency)
	
	int bufferSize		= 512;
	sampleRate 			= 44100;
	
	phase1				= 0;
	phase2				= 0;
	phase3				= 0;
	
	shape1				= 0;
	shape2				= 0;
	shape3				= 0;
	
	freeze1				= 0;
	freeze2				= 0;
	freeze3				= 0;
	
	phaseAdder 			= 0.0f;
	volume				= 0.2f;
	bNoise 				= false;
	
	voice1.assign(bufferSize, 0.0);
	voice2.assign(bufferSize, 0.0);
	voice3.assign(bufferSize, 0.0);
	
	voices.push_back(&voice1);
	voices.push_back(&voice2);
	voices.push_back(&voice3);
		
	frequency = targetFrequency = 110;
	numerator1 = numerator2 = 2;
	denominator1 = denominator2 = 3;
	rotator = 0;
	
	//soundStream.listDevices();
	
	//if you want to set the device id to be different than the default
	//soundStream.setDeviceID(1); 	//note some devices are input only and some are output only
	
	soundStream.setup(this, 2, 0, sampleRate, bufferSize, 4);
	
	// fft
	
	plotHeight = 100;
	
	//fft = ofxFft::create(bufferSize, OF_FFT_WINDOW_HAMMING, OF_FFT_FFTW);
	fft = ofxFft::create(bufferSize, OF_FFT_WINDOW_HAMMING);
	
	drawBins.resize(fft->getBinSize());
	middleBins.resize(fft->getBinSize());
	audioBins.resize(fft->getBinSize());
	
	// history
	
	hwidthi = 256;
	hwidthf = hwidthi;
	
	history.setMode(OF_PRIMITIVE_LINE_STRIP);
	
	for(int y = 0; y < hwidthi; y++ )
	{
		for(int x = 0; x < hwidthi; x++ )
		{
			ofFloatColor color(245.f/255.f, 58.f/255.f, 135.f/255.f,y/255.f);
			history.addColor(color);
			ofVec3f pos(x-hwidthf/2.f, y-hwidthf/2.f, 0);
			history.addVertex(pos);
		}
	}
	
	historyIndex = 0;
	historyZBuffer.assign(hwidthi*hwidthi, 0);
	historyCBuffer.assign(hwidthi*hwidthi, 0);
	
	cam.enableOrtho();
}


//--------------------------------------------------------------
void hsApp::update()
{
}

//--------------------------------------------------------------
void hsApp::draw()
{
	ofBackground(34, 34, 34);
	
	int W = 256;
	int H = 64;
	
	vector<ofVec3f>& verts = mesh.getVertices();
	vector<ofFloatColor>& color = mesh.getColors();
	
	for(unsigned int i = 0; i < verts.size(); i++)
	{
		int j = ofMap(i, 0, verts.size(), 0, widthf);
		
		verts[i].x = voice1[j] * W;
		verts[i].y = voice2[j] * W;
		verts[i].z = voice3[j] * W;
		
		color[i].r = 1.f - voice1[j];
		color[i].g = 1.f - voice2[j];
		color[i].b = 1.f - voice3[j];
	}
	
	ofPushMatrix();
	ofTranslate(512, 528, 0);
	ofRotateY(pan*360);
	ofRotateX(rotator);
	rotator += 0.1f;
	glPointSize(3);
	mesh.draw();
	ofPopMatrix();
	
	//
	
	ofSetColor(225);
	ofDrawBitmapString("HyperScope", 64, 64);
//	ofDrawBitmapString("press 'z' to unpause the audio, press 'x' to pause the audio", 32, 92);
	
	ofNoFill();

	// draw voices
	
	ofPushStyle();
	ofPushMatrix();
	ofTranslate(64, 128, 0);
	
	for(vector<float>::size_type i = 0; i != voices.size(); i++)
	{
		vector<float> voice = *voices[i];
		
		ofSetColor(225);
		ofDrawBitmapString("Voice " + ofToString(i+1), 0, 16);
		
		ofSetLineWidth(1);
		//ofRect(0, 0, W, H);
		
		ofSetColor(245, 58, 135);
		ofBeginShape();
		for (unsigned int j = 0; j < voice.size(); j++)
		{
			float x = ofMap(j, 0, voice.size(), 0, W, true);
			ofVertex(x, H/2 - voice[j]*80.0f);
		}
		ofEndShape(false);
		
		ofTranslate(0, H, 0);
	}
	
	ofPopMatrix();
	ofPopStyle();
	
	// draw output
	
	ofPushStyle();
	ofPushMatrix();
	ofTranslate(64, 368, 0);
	
	ofSetColor(225);
	ofDrawBitmapString("Output", 0, 16);
	
	ofSetLineWidth(1);
	//ofRect(0, 0, W, 100);
	
	ofSetColor(245, 58, 135);
	ofSetLineWidth(1);
	
	ofBeginShape();
	for (unsigned int i = 0; i < voice3.size(); i++)
	{
		float x =  ofMap(i, 0, voice3.size(), 0, W, true);
		ofVertex(x, H/2 -(voice1[i]+voice2[i]+voice3[i])*80.0f);
	}
	ofEndShape(false);
	
	ofPopMatrix();
	ofPopStyle();
	
	// again
	
	ofPushStyle();
	ofPushMatrix();
	ofTranslate(512, 256, 0);
	ofRotateY(pan*360);
	ofRotateX(rotator);
	
	ofSetColor(245, 58, 135);
	ofSetLineWidth(1);
	
	ofBeginShape();
	for (unsigned int i = 0; i < voice3.size(); i++)
	{
		ofVertex(voice1[i]*W, voice2[i]*W, voice3[i]*W);
	}
	ofEndShape(false);
	
	ofPopMatrix();
	ofPopStyle();
	
	// draw info
	
	ofSetColor(225);
	string reportString = "volume: ("+ofToString(volume, 2)+") modify with -/+ keys\n";//pan: ("+ofToString(pan, 2)+") modify with mouse x\nsynthesis: ";
	if( !bNoise )
	{
		reportString += "sine wave (" + ofToString(frequency, 2) + " > " + ofToString(targetFrequency, 2) + "hz) modify with mouse y";
	}
	else
	{
		reportString += "noise";
	}
	reportString += "\nratios = " + ofToString(numerator1) + ":" + ofToString(denominator1) + ", " + ofToString(numerator2) + ":" + ofToString(denominator2);
	
	reportString += "\nchange ratios: 'q/a':'w/s', 'e/d':'r/f'";
	reportString += "\nchange waveforms: 'y/h/n'";
	reportString += "\nfreeze waveforms: 'u/j/m'";
	
	ofDrawBitmapString(reportString, 32, 579);
	
	// fft
	
	ofSetColor(255);
	ofPushMatrix();
	ofTranslate(704, 128);
	
	soundMutex.lock();
	drawBins = middleBins;
	soundMutex.unlock();
	
	plot(drawBins, -plotHeight, 304+plotHeight / 2);
	ofPopMatrix();
	//string msg = ofToString((int) ofGetFrameRate()) + " fps";
	//ofDrawBitmapString(msg, ofGetWidth() - 80, ofGetHeight() - 20);
	
	//
	
	vector<ofVec3f>& hverts = history.getVertices();
	vector<ofFloatColor>& hcolor = history.getColors();
	
	int y = historyIndex++;
	if( historyIndex >= hwidthi ) historyIndex = 0;
	for(int x = 0; x < hwidthi; x++ )
	{
		int j = ofMap(x, 0, hwidthi, 0, drawBins.size()/2);
		historyZBuffer[x + y*hwidthi] = sqrt(drawBins[j]) * hwidthf;
		historyCBuffer[x + y*hwidthi] = sqrt(drawBins[j]);
	}
	
	for(int y = 0; y < hwidthi; y++ )
	{
		for(int x = 0; x < hwidthi; x++ )
		{
			int bufferIndex = (historyIndex + y) % hwidthi;
			hverts[x + y*hwidthi].z = historyZBuffer[x + bufferIndex*hwidthi];
			hcolor[x + y*hwidthi].r = historyCBuffer[x + bufferIndex*hwidthi]*245.f/255.f;
			hcolor[x + y*hwidthi].g = historyCBuffer[x + bufferIndex*hwidthi]*58.f/255.f;
			hcolor[x + y*hwidthi].b = historyCBuffer[x + bufferIndex*hwidthi]*145.f/255.f;
		}
	}
	
	cam.begin();
	ofScale(1,-1,1);
	ofTranslate(832, -512);
		
	ofPushMatrix();
	ofRotateX(25);
	glPointSize(1);
	history.draw();
	ofPopMatrix();
	
	cam.end();
}

//--------------------------------------------------------------
void hsApp::plot(vector<float>& buffer, float scale, float offset)
{
	ofNoFill();
	int n = buffer.size()/2;
	//ofRect(0, 0, n, plotHeight);
	glPushMatrix();
	glTranslatef(0, plotHeight / 2 + offset, 0);
	ofBeginShape();
	for (int i = 0; i < n; i++)
	{
		ofVertex(2*i, sqrt(buffer[i]) * scale);
	}
	ofEndShape();
	glPopMatrix();
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
	
	if( key == 'z' )
	{
		soundStream.start();
	}
	
	if( key == 'x' )
	{
		soundStream.stop();
	}
	
	if( key == 'a' )
	{
		numerator1--;
		if( numerator1 < 1 ) numerator1 = 1;
	}
	
	if( key == 'q' )
	{
		numerator1++;
	}
	
	if( key == 's' )
	{
		denominator1--;
		if( denominator1 < 1 ) denominator1 = 1;
	}
	
	if( key == 'w' )
	{
		denominator1++;
	}
	
	if( key == 'd' )
	{
		numerator2--;
		if( numerator2 < 1 ) numerator2 = 1;
	}
	
	if( key == 'e' )
	{
		numerator2++;
	}
	
	if( key == 'f' )
	{
		denominator2--;
		if( denominator2 < 1 ) denominator2 = 1;
	}
	
	if( key == 'r' )
	{
		denominator2++;
	}
	
	int N_SHAPES = 3;
	
	if( key == 'y' )
	{
		shape1 = (shape1 + 1) % N_SHAPES;
	}
	if( key == 'h' )
	{
		shape2 = (shape2 + 1) % N_SHAPES;
	}
	if( key == 'n' )
	{
		shape3 = (shape3 + 1) % N_SHAPES;
	}
	
	if( key == 'u' )
	{
		freeze1 = freeze1 ? 0 : 1;
	}
	if( key == 'j' )
	{
		freeze2 = freeze2 ? 0 : 1;
	}
	if( key == 'm' )
	{
		freeze3 = freeze3 ? 0 : 1;
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
			phase1 += freeze1 ? 0 : phaseAdder;
			phase2 += freeze2 ? 0 : phaseAdder*float(denominator1)/float(numerator1);
			phase3 += freeze3 ? 0 : (phaseAdder*float(denominator1)/float(numerator1))*float(denominator2)/float(numerator2);
			
			// sine wave
			float sample1 = sin(phase1);
			float sample2 = sin(phase2);
			float sample3 = sin(phase3);
			
			// square wave
			if( shape1 == 1 ) sample1 = sample1 > 0 ? 1 : -1;
			if( shape2 == 1 ) sample2 = sample2 > 0 ? 1 : -1;
			if( shape3 == 1 ) sample3 = sample3 > 0 ? 1 : -1;
			
			// sawtooth wave
			if( shape1 == 2 ) sample1 = (fmodf(phase1,TWO_PI) - PI)/2.f;
			if( shape2 == 2 ) sample2 = (fmodf(phase2,TWO_PI) - PI)/2.f;
			if( shape3 == 2 ) sample3 = (fmodf(phase3,TWO_PI) - PI)/2.f;
			
			// scale by volume
			voice1[i] = sample1 * volume;
			voice2[i] = sample2 * volume;
			voice3[i] = sample3 * volume;
			
			output[i*nChannels] = output[i*nChannels + 1] = voice1[i] + voice2[i] + voice3[i];
		}
	}
	
	// fft

	fft->setSignal(output);
	
	float* curFft = fft->getAmplitude();
	memcpy(&audioBins[0], curFft, sizeof(float) * fft->getBinSize());
		
	soundMutex.lock();
	middleBins = audioBins;
	soundMutex.unlock();	
}

//--------------------------------------------------------------
void hsApp::gotMessage(ofMessage msg)
{
}

//--------------------------------------------------------------
void hsApp::dragEvent(ofDragInfo dragInfo)
{
}
