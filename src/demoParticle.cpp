#include "demoParticle.h"

//------------------------------------------------------------------
demoParticle::demoParticle(){
}



//------------------------------------------------------------------
void demoParticle::reset(){
   // attractPt = ofPoint(ofRandom(0, ofGetWidth()), ofRandom(0, ofGetHeight()));
	pos.x = ofRandom(1280);
	pos.y = ofRandom(screenHeight) * 0.75;
	
	vel.x = ofRandom(-3.9, 3.9);
	vel.y = ofRandom(-3.9, 3.9);
	
	frc   = ofPoint(0,0,0);
	
		drag  = ofRandom(0.97, 0.998);
    
    startTime = ofGetElapsedTimeMillis();  // get the start time
    endTime = (int)ofRandom(1000, 5000); // in milliseconds
    
	
}

//------------------------------------------------------------------
void demoParticle::update(){

	//1 - APPLY THE FORCES BASED ON WHICH MODE WE ARE IN
    
    // update the timer this frame
    float timer = ofGetElapsedTimeMillis() - startTime;
    
    if(timer >= endTime) {
        int ranx = ofRandom(0, screenWidth);
        int rany = ofRandom(0, screenHeight * 0.75);
        attractPt = ofPoint(ranx, rany);
        startTime = ofGetElapsedTimeMillis();  // get the start time
        endTime = (int)ofRandom(6000, 8000); // in milliseconds
    }
    
        frc = attractPt-pos; // we get the attraction force/vector by looking at the mouse pos relative to our pos
		frc.normalize(); //by normalizing we disregard how close the particle is to the attraction point
    vel *= drag; //apply drag
    vel += frc * 0.3; //apply force
    
  
		
    if( pos.x < 100 || pos.x > screenWidth - 100 || pos.y < 100 || pos.y > screenHeight* 0.75 - 100){
        vel *= 0.965; //
    }
	
	//2 - UPDATE OUR POSITION
	
	pos += vel/8;
	
	
	//3 - (optional) LIMIT THE PARTICLES TO STAY ON SCREEN 
	//we could also pass in bounds to check - or alternatively do this at the ofApp level
	if( pos.x > screenWidth ){
		pos.x = screenWidth;
		vel.x *= -1.0;
	}else if( pos.x < 0 ){
		pos.x = 0;
		vel.x *= -1.0;
	}
	if( pos.y > screenHeight*0.75 ){
		pos.y = screenHeight*0.75;
		vel.y *= -1.0;
	}
	else if( pos.y < 0 ){
		pos.y = 0;
		vel.y *= -1.0;
	}	
		
}

//------------------------------------------------------------------
void demoParticle::draw(){

	ofCircle(pos.x, pos.y, 5);
 //   ofSetColor(0, 0, 180);

 //   ofCircle(attractPt.x, attractPt.y, 20);

}

