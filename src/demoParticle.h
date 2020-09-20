#pragma once
#include "ofMain.h"

enum particleMode{
	PARTICLE_MODE_ATTRACT = 0,
	PARTICLE_MODE_REPEL,
	PARTICLE_MODE_NEAREST_POINTS,
	PARTICLE_MODE_NOISE
};

class demoParticle{

	public:
		demoParticle();
		
		void reset();
		void update();
		void draw();		
		
		ofPoint pos;
		ofPoint vel;
		ofPoint frc;
		
		float drag; 
        int screenHeight = 720;
        int screenWidth = 1280;
		particleMode mode;
		
		vector <ofPoint> * attractPoints;
    
    float startTime; // store when we start time timer
    float endTime; // when do want to stop the timer
    
    ofPoint attractPt;
    };