#pragma once

#include "ofxOpenNI.h"
#include "ofMain.h"
#include "ofxVideoRecorder.h"
#include "demoParticle.h"
#include "ofxUI.h"


#define MAX_DEVICES 2

class ofApp : public ofBaseApp{
    
public:
    
    bool debug;
    
    // Jenna UI
    void launchGUI();
    void guiEvent(ofxUIEventArgs &e);
    
    
    // Enviroment
    int kinectX = 640;
    int kinectY = 480;
   
    int framerate = 30;
    float DALScreenAdjust = 0.75;
    // Interaction Variables
    float x = 0;
    float y = 0;
    float w = 300;
    float h = 200;
    bool interacting = false;
    bool interactionStarted = false;
    ofRectangle interactionWindow;
    ofImage videoFeed;
    
    // Hands
    ofPoint hand1pos;
    ofPoint hand2pos;
    
    // Interaction Config
    // int resizeMaxWidth = 1464;
    // int resizeMaxHeight = 883;
    int resizeMaxWidth = 640;
    int resizeMaxHeight = 480;
    int trimX = -150;
    int trimY = -200;
    
    
    // Video
    vector<ofVideoDevice> videoDevices;
    ofVideoGrabber      vidGrabber; //Gets webcam
    ofxVideoRecorder	vidRecorder; // Record the videp
    
    ofVideoPlayer       playMovie; // Plays video
    
    
    //ofxVideoRecorder    vidRecorder;
    ofSoundStream       soundStream;
    bool isRecording;
    int sampleRate;
    int channels;
    string fileName;
    string fileExt;
    ofFbo recordFbo;
    ofPixels recordPixels;
    ofImage recordingWindow;
    
    bool vidSaved = false;
  //  int camDimHeight = 720;
  //  int camDimWidth = 1280;
    string vidName; //Saved video name
    
    
    ofImage vidImage;
    
    
    ////////////////////////////////////// Adjustable varaibles //////////////////////////////////////
    // Variables to include
    
    Boolean debugMode = false;              // Allow setting to change
    
    int debugPanelW = 420;
    int debugPanelH = ofGetHeight() * DALScreenAdjust;
    
    //GUI for debug / config mode
    ofxUISuperCanvas *gui1;

    // Kinect depth
    int nearThreshold = 2000;
    int farThreshold = 3400;
    // int guiDepthMin, guiDepthMax;
    
    
    // Basic dimensions
    // float screenAdjust = 0.75;
    int resX = 1280; // This is the resolution everything is being made at
    int resY = 720; // * 0.75
    
    int screenStartTime = 0; // store when we start time timer
    int screenEndTime = 8000; // when do want to stop the timer
    
    //////////////////////////////////////////////////////////////////////////////////////////////////
    
    
    // OpenFrameworks Methods
	void setup();
	void update();
	void draw();
    void exit();
	void keyPressed  (int key);
	void keyReleased(int key);
	void mouseMoved(int x, int y );
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
	void windowResized(int w, int h);
    void dragEvent(ofDragInfo dragInfo);
    void gotMessage(ofMessage msg);
    void audioIn(float * input, int bufferSize, int nChannels);
    
    // Digital Canvas Methods
    ofPoint coordinateProcessor(ofPoint);
    void interactionPreProcessor(float dotPos1X, float dotPos1Y, float dotPos2X, float dotPos2Y);
    void updateInteraction(float dotPos1X, float dotPos1Y, float dotPos2X, float dotPos2Y);
    void drawInteraction();
    
    void drawHands();
    void startRecording();
    void updateRecorder();
    void stopRecording();
    void saveVideo();
    void swapVideos();
    void playInteractions();
    void resizeGesture();
    void moveGesture();
    void displaySilhouette();
    void displayDebug();
    void updateCoordinates();
    void screenSaverIndicator(int posX, int posY, int radius); // Indicator

    
    // New Variables
    int recordingStartTime = 0;
    ofImage userMask;
    
    SceneMetaData sceneMetaData;
    unsigned short* depthPixels;
    
    
    
    ofShader    shader;
    ofPixels maskPixels;
    
    ofImage maskImage;
    ofxOpenNI openNIDevice;
    // Events
    void userEvent(ofxOpenNIUserEvent & event);
    void gestureEvent(ofxOpenNIGestureEvent & event);
    
    
    
    unsigned char *		depth_pixels;
    
    
    
    //Touch events
    //    void TouchDown(Blob b);
    //    void TouchMoves(Blob b);
    //    void TouchUp(Blob b);
    
    
    //FBO for webcam
    ofFbo       maskFbo;
    ofFbo       fbo;
    
    
    
    //webCam
    //ofVideoGrabber VidGrabber;
    
    //recorder
    //ofPtr<ofQTKitGrabber> fakeRecorder; //This is still needed for vid recorder to work
    
    
    
    //Video
    ofVideoPlayer MemoryPlayback;
    
    
    //blending
    ofBlendMode blendMode;
    
    // Check video should record once
    bool bRecording = false;
    
    //ofFbo recordFbo;
    // ofPixels recordPixels;
    
    int iVidPlayCount = 0;
    int iTotalVideos = 3;
    bool bUpperHalf = false;
    
    // Video recording variables
    /////// HIT LIST ////////
    int iInteractionTimer = 0;
    int iStartTimer = 0;
    int iVidTimer = 0;
    int iTotalVidTime = 0;
    
    bool bIncludeVid = false;
    bool bVidRecording = false;
    bool b0Record = true;
    
    //Hand tracking event
    
private:
    
	
    ofTrueTypeFont verdana;
    
    
    /////Screen Saver
    vector <demoParticle> fakeHands;
    
    
    bool startScreenSaver = false;
    
    int screenSaverAlpha = 0;
    int radiusSize = 3;
    
};
