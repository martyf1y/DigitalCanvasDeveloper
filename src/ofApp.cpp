#include "ofApp.h"
#include "stdio.h"
#include "ofUtils.h"
#include "global.h"
//--------------------------------------------------------------
void ofApp::setup() {
    
    // Setup Environment Variables
    ofEnableAlphaBlending();
    ofEnableSmoothing();
    // ofSetFrameRate(framerate);
    ofSetLogLevel(OF_LOG_VERBOSE);
    ofLogToConsole();
    
    //fbo
    maskFbo.allocate(resX,resY);
    fbo.allocate(resX,resY);
    
    if (debugMode) {
        launchGUI();
    } else {
        //hide cursor
        ofHideCursor();
    }
    
    /// Screensaver
    fakeHands.push_back(demoParticle());
    fakeHands.push_back(demoParticle());
    
	fakeHands[0].reset();
    fakeHands[1].reset();
    
#ifdef TARGET_OPENGLES
    shader.load("shaders_gles/alphamask.vert","shaders_gles/alphamask.frag");
#else
    if(ofGetGLProgrammableRenderer()){
    	string vertex = "#version 150\n\
    	\n\
		uniform mat4 projectionMatrix;\n\
		uniform mat4 modelViewMatrix;\n\
    	uniform mat4 modelViewProjectionMatrix;\n\
    	\n\
    	\n\
    	in vec4  position;\n\
    	in vec2  texcoord;\n\
    	\n\
    	out vec2 texCoordVarying;\n\
    	\n\
    	void main()\n\
    	{\n\
        texCoordVarying = texcoord;\
        gl_Position = modelViewProjectionMatrix * position;\n\
    	}";
		string fragment = "#version 150\n\
		\n\
		uniform sampler2DRect tex0;\
		uniform sampler2DRect maskTex;\
        in vec2 texCoordVarying;\n\
		\
        out vec4 fragColor;\n\
		void main (void){\
		vec2 pos = texCoordVarying;\
		\
		vec3 src = texture(tex0, pos).rgb;\
		float mask = texture(maskTex, pos).r;\
		\
		fragColor = vec4( src , mask);\
		}";
		shader.setupShaderFromSource(GL_VERTEX_SHADER, vertex);
		shader.setupShaderFromSource(GL_FRAGMENT_SHADER, fragment);
		shader.bindDefaults();
		shader.linkProgram();
    }else{
		string shaderProgram = "#version 120\n \
		#extension GL_ARB_texture_rectangle : enable\n \
		\
		uniform sampler2DRect tex0;\
		uniform sampler2DRect maskTex;\
		\
		void main (void){\
		vec2 pos = gl_TexCoord[0].st;\
		\
		vec3 src = texture2DRect(tex0, pos).rgb;\
		float mask = texture2DRect(maskTex, pos).r;\
		\
		gl_FragColor = vec4( src , mask);\
		}";
		shader.setupShaderFromSource(GL_FRAGMENT_SHADER, shaderProgram);
		shader.linkProgram();
    }
#endif
    
    
    maskFbo.begin();
    ofClear(0,0,0,255);
    maskFbo.end();
    
    fbo.begin();
    ofClear(0,0,0,255);
    fbo.end();
    
    // Video Configuration
    vidGrabber.setDesiredFrameRate(30);
    vidGrabber.initGrabber(resX, resY);
    //    vidRecorder.setFfmpegLocation(ofFilePath::getAbsolutePath("ffmpeg")); // use this is you have ffmpeg installed in your data folder
    
    fileName = "testMovie";
    fileExt = ".mov"; // ffmpeg uses the extension to determine the container type. run 'ffmpeg -formats' to see supported formats
    
    // override the default codecs if you like
    // run 'ffmpeg -codecs' to find out what your implementation supports (or -formats on some older versions)
    vidRecorder.setVideoCodec("mpeg4");
    vidRecorder.setVideoBitrate("800k");
    // vidRecorder.setAudioCodec("mp3");
    // vidRecorder.setAudioBitrate("192k");
    
    //    soundStream.listDevices();
    //    soundStream.setDeviceID(11);
    soundStream.setup(this, 0, channels, sampleRate, 256, 4);
    
    recordFbo.allocate(resX, resY, GL_RGB);
    recordPixels.allocate(resX, resY, OF_IMAGE_COLOR);
    
    // ofSetWindowShape(vidGrabber.getWidth(), 768);
    
    // OpenNI Setup
    openNIDevice.setup();
    
    openNIDevice.getDepthTextureReference();
    openNIDevice.getFrameRate();
    
    openNIDevice.addImageGenerator();
    openNIDevice.addDepthGenerator();
    openNIDevice.addGestureGenerator();
    //   openNIDevice.setRegister(true);
    openNIDevice.setMirror(true);
    openNIDevice.addHandsGenerator();
    openNIDevice.setMaxNumHands(2);
    
    openNIDevice.addAllGestures();
    // openNIDevice.addUserGenerator();
    
    openNIDevice.start();
    
    // Resources
    verdana.loadFont(ofToDataPath("verdana.ttf"), 24);
    openNIDevice.globalMinDepth = nearThreshold;                               // Set initial depths
    openNIDevice.globalMaxDepth = farThreshold;
}

//--------------------------------------------------------------
void ofApp::update(){
    
    // Inputs
    // cout << "HERE1" << endl;
    
    openNIDevice.update();
    // cout << "HERE2" << endl;
    
    
    if (debugMode) {
        
        //UPDATE VALUES TO-FROM GUI SLIDERS
        
        // Round Kinect Min-Max depth to nearest 100:
        // minDepthSetter = (guiDepthMin + 50) / 100 * 100;
        // maxDepthSetter = (guiDepthMax + 50) / 100 * 100;
        nearThreshold = (nearThreshold + 50) / 100 * 100;
        farThreshold = (farThreshold + 50) / 100 * 100;
        
        //round Dimensions to nearest 10 pixels: (second)
        resX = (resX + 5) / 10 * 10;
        resY = (resY + 5) / 10 * 10;
        
        //update Kinect with new depths
        openNIDevice.setMinDepth(nearThreshold);
        openNIDevice.setMaxDepth(farThreshold);
    }

    
    vidGrabber.update();
    if(vidSaved){
        
        if(playMovie.isLoaded()){
            playMovie.play();
            vidSaved = false;
        }
        else{
            playMovie.loadMovie(vidName);
        }
    }
    if(playMovie.isLoaded()){
        playMovie.update();
    }
    
    
    //updateRecorder();
    //displaySilhouette();
    /*  code to use
     openNIDevice.getDepthRawPixels()
     */
    
    //Matt this updates the mask so only its position is revealed
    maskFbo.begin(); //Turn this off to see a cool spray paint effect
    ofClear(0,0,0,255);
    maskFbo.end();
    
    // Outputs
    updateCoordinates();
    
    if(screenSaverAlpha > 0){
        
        fakeHands[0].update();
        fakeHands[1].update();
        
        interactionPreProcessor(fakeHands[0].pos.x, fakeHands[0].pos.y, fakeHands[1].pos.x, fakeHands[1].pos.y);
        
        updateInteraction(fakeHands[0].pos.x, fakeHands[0].pos.y, fakeHands[1].pos.x, fakeHands[1].pos.y);
    }
    //cout << "HERE4" << endl;
    
    if(interacting){
        startScreenSaver = false;
        screenSaverAlpha = 0;
        updateInteraction(hand1pos.x, hand1pos.y, hand2pos.x, hand2pos.y);
        
        if(!bRecording && !vidRecorder.isInitialized()){
            interactionStarted = true;
            vidName = fileName+ofGetTimestampString()+fileExt;
            //  vidRecorder.setup(vidName, dimWidth, dimHeight, 30, sampleRate, channels);
            vidRecorder.setup(vidName, resX, resY, 30); // no audio
            
            //    vidRecorder.setupCustomOutput(vidGrabber.getWidth(), vidGrabber.getHeight(), 30, sampleRate, channels, "-vcodec mpeg4 -b 1600k -acodec mp2 -ab 128k -f mpegts udp://localhost:1234"); // for custom ffmpeg output string (streaming, etc)
            
        }
        bRecording = true;
    }
    else if (interactionStarted){
        bRecording = false;
        vidRecorder.close();
        playMovie.close();
        playMovie.loadMovie(vidName);
        vidSaved = true;
        interactionStarted = false;
    }
    else if (!interacting){
        // update the timer this frame
        float timer = ofGetElapsedTimeMillis() - screenStartTime; // Put this when there is no interaction
        
        if(screenEndTime < ofGetElapsedTimeMillis() - screenStartTime){
            
            startScreenSaver = !startScreenSaver;
            screenStartTime = ofGetElapsedTimeMillis();  // get the start time
        }
        
    }
    cout << "interacting: " << interacting << endl;
    
    if(startScreenSaver && screenSaverAlpha < 255){
        screenSaverAlpha ++;
        screenEndTime = 15000;
    }
    else if (!startScreenSaver && screenSaverAlpha > 0){
        screenSaverAlpha --;
        screenEndTime = 8000;
    }
    
    
    // HERE the shader-masking happends
    //
    fbo.begin();
    // Cleaning everthing with alpha mask on 0 in order to make it transparent for default
    ofClear(0, 0, 0, 0);
    
    shader.begin();
    shader.setUniformTexture("maskTex", maskFbo.getTextureReference(), 1 );
    
    vidGrabber.draw(0,0);
    
    shader.end();
    fbo.end();
    
    recordFbo.begin();
    ofBackground(0);

    
    ofFill();
    if(playMovie.isLoaded()){
        playMovie.draw(0, 0, resX, resY);
    }
    if(interacting){
        fbo.draw(0,0);
    }
    // playInteractions();
    
    recordFbo.end();
    
    
    if(vidGrabber.isFrameNew() && bRecording){
        // vidRecorder.addFrame(vidGrabber.getPixelsRef());
        recordFbo.readToPixels(recordPixels);
        vidRecorder.addFrame(recordPixels);
    }
}

//--------------------------------------------------------------
void ofApp::draw(){
    ofSetColor(180, 180, 180);
    ofRect(0, 720*DALScreenAdjust, ofGetWidth(), 720-(720*DALScreenAdjust));
    ofSetColor(255, 255, 255);
    
    recordFbo.draw(0,0, 1366, 720*DALScreenAdjust); // This is what to be saved into the video, it also draws onto screen
    openNIDevice.drawDepth(-20,0, 1386, 720*DALScreenAdjust);
    ofSetColor(255, 255, 255, 150);
    
    
    // openNIDevice.drawImage(0,0, 1366, 768*0.75);
    ofSetColor(255, 255, 255, 255);
    

    drawHands();
    
     ofPushStyle();
    if(screenSaverAlpha > 0){
       
        
        ofSetColor(255, 255, 255, screenSaverAlpha);
        ofNoFill();
        // THIS IS THE BORDER OF THE INTERACTION
        
        int fakeYPos =0;
        int fakeXPos = fakeHands[0].pos.x;
        
        if(fakeHands[0].pos.y < fakeHands[1].pos.y){ // If the hands have swapped
            fakeYPos = fakeHands[0].pos.y;
        }
        else{
            fakeYPos = fakeHands[0].pos.y-h;
        }
        if(w>0){
            ofRect(fakeXPos-1, fakeYPos-1, w+2, h+2);
            ofRect(fakeXPos-2, fakeYPos-2, w+4, h+4);
            ofRect(fakeXPos-3, fakeYPos-3, w+6, h+6);
        }
        else{
            ofRect(fakeXPos+1, fakeYPos-1, w-2, h+2);
            ofRect(fakeXPos+2, fakeYPos-2, w-4, h+4);
            ofRect(fakeXPos+3, fakeYPos-3, w-6, h+6);
        }
        ofFill();
        
        fbo.draw(0,0);
        fakeHands[0].draw();
        fakeHands[1].draw();
        
        if((screenSaverAlpha >= 255 || radiusSize < 1200)){ // Indicator
            for(int i = 0; i < 5; i++){
                screenSaverIndicator(fakeHands[0].pos.x, fakeHands[0].pos.y, radiusSize/3 + 10 - (i*5));
                screenSaverIndicator(fakeHands[1].pos.x, fakeHands[1].pos.y, radiusSize/3 + 10 - (i*5));
            }
            radiusSize +=3;
        }
        else{
            radiusSize = 0;
        }
    }
    ofPopStyle();
    if (debugMode) {
        //Display debug stats
        ofPushStyle();
        displayDebug();
        ofPopStyle();
    }
    
    // Start User Interaction
    //   if(openNIDevice.getNumTrackedHands() > 0){
    
    //     if(openNIDevice.getNumTrackedHands()==1)
    //         moveGesture();
    
    
    //     if(openNIDevice.getNumTrackedHands()==2)
    //        resizeGesture();
    
    //
    // }
    //  else
    //  {
    // No hands - reset interaction
    
    //      if(openNIDevice.getNumTrackedUsers() > 0)
    //          openNIDevice.resetUserTracking(0);
    //  }
    
    
    //  }
}
//--------------------------------------------------------------
void ofApp::exit(){
    openNIDevice.stop();
    vidRecorder.close();
    
    vidGrabber.close();
    if (MemoryPlayback.isLoaded()) {
        MemoryPlayback.close();
    }
}

void ofApp::audioIn(float *input, int bufferSize, int nChannels){
    if(bRecording)
        vidRecorder.addAudioSamples(input, bufferSize, nChannels);
}

void ofApp::screenSaverIndicator(int posX, int posY, int radius){ // Indicators
    ofPushStyle();
    ofSetColor(255, 255, 255, 205 - radius*3);
    ofCircle(posX, posY, radius);
    ofPopStyle();
    // ofNoFill();
    // ofCircle(posX, posY, radius - 10);
    // ofFill();
}


//--------------------------------------------------------------
// Update Methods
//--------------------------------------------------------------
/*
 Sets Coordinates For Each Hand
 */
void ofApp::updateCoordinates(){
    int numHands = openNIDevice.getNumTrackedHands();
    
    if(numHands > 0){
        interacting = true;
        
        if(numHands == 1){
            // Get Hand
            ofxOpenNIHand & hand = openNIDevice.getTrackedHand(0);
            
            if(w < 200){w = 200;}// Stop things from getting too small
            if(h < 200){h = 200;}
            
            hand1pos.set(coordinateProcessor(hand.getPosition()));
            // hand2pos.zero(); // null inactive hand
            hand2pos.x = 0; // null inactive hand
            hand2pos.y = 0;
        }
        if(numHands == 2){
            // Get Hands
            ofxOpenNIHand & handA = openNIDevice.getTrackedHand(0);
            ofxOpenNIHand & handB = openNIDevice.getTrackedHand(1);
            
            // Find Hand 1 (Hand 1 must always be to the left of the interaction window)
            if(handA.getPosition().x < handB.getPosition().x){
                // HandA = hand1
                hand1pos.set(coordinateProcessor(handA.getPosition()));
                hand2pos.set(coordinateProcessor(handB.getPosition()));
            }
            else {
                // HandB = hand1
                hand1pos.set(coordinateProcessor(handB.getPosition()));
                hand2pos.set(coordinateProcessor(handA.getPosition()));
            }
        }
        // Run interaction pre processor
        interactionPreProcessor(hand1pos.x, hand1pos.y, hand2pos.x, hand2pos.y); //  Sets interaction window size
    }
    else{
        // No hands (null each hand)
        interacting = false;
        hand1pos.zero();
        hand2pos.zero();
        hand1pos.x = 0; // null inactive hand
        hand1pos.y = 0;
        
        hand2pos.x = 0; // null inactive hand
        hand2pos.y = 0;
        if(openNIDevice.getNumTrackedHands()>0){
            openNIDevice.removeHandsGenerator();
            openNIDevice.addHandsGenerator();
        }
    }
}

/*
 Modifies Hand Positions To Consider Interaction Size
 */
void ofApp::interactionPreProcessor(float dotPos1X, float dotPos1Y, float dotPos2X, float dotPos2Y){
    
    if(dotPos2X != 0 && dotPos2Y != 0){
        // Set Interaction Window Size
        w = round(dotPos2X - dotPos1X);
        
        if(dotPos1Y > dotPos2Y){ // Check if the y axis is inverted
            h = round(dotPos1Y - dotPos2Y);
        }
        else{
            h = round(dotPos2Y - dotPos1Y);
        }
    }
}


void ofApp::updateInteraction(float dotPos1X, float dotPos1Y, float dotPos2X, float dotPos2Y){
    
    maskFbo.begin();
    if(dotPos1Y < dotPos2Y){ // If the hands have swapped
        ofRect(dotPos1X, dotPos1Y, w, h);
    }
    else if (openNIDevice.getNumTrackedHands()==1){ // If there is one hand
        ofRect(dotPos1X-w/2, dotPos1Y-h/2, w, h);
    }
    else{
        ofRect(dotPos1X, dotPos1Y-h, w, h); // If two hands
    }
    maskFbo.end();
    
    
}

void ofApp::updateRecorder(){
    
    
}

// Resize Gesture
void ofApp::resizeGesture(){
    // if(!interacting) interacting = true; // Start Interaction
    
    // Resize Interaction Window
    if(hand1pos.y < hand2pos.y)//Matt put this to autocorrect position of window
    {
        //interactionWindow.set(hand1pos.x, hand1pos.y, w, h);
        
        // ofRect(hand1pos.x, hand1pos.y, w, h);
    }
    else{
        //interactionWindow.set(hand1pos.x, hand1pos.y-h, w, h);
        //ofRect(hand1pos.x, hand1pos.y-h, w, h);
    }
}

// Move Gesture
void ofApp::moveGesture(){
    if(interacting){ // Check if interacting
        
        // Move Interaction Window
        //  interactionWindow.set(hand1pos.x, hand1pos.y, w, h);
    }
    else{
        cout << "Move Gesture Without Interaction\n";
        ofLog(OF_LOG_NOTICE, "Move Gesture Without Interaction");
    }
}


//--------------------------------------------------------------
// Draw Methods
//--------------------------------------------------------------


/*
 Draws the position of the users hands onscreen
 */
void ofApp::drawHands(){
    ofPushStyle();
    
    // THIS IS THE INTERACTING BORDER. IT IS RED.
    ofNoFill();
    if(interacting){
        int handYPos =0;
        int handXPos = hand1pos.x/resX*1366;
        
        if(hand1pos.y < hand2pos.y){ // If the hands have swapped
            handYPos = hand1pos.y/resY*720*DALScreenAdjust;
        }
        else if (openNIDevice.getNumTrackedHands()==1){ // If there is one hand
            handYPos = (hand1pos.y-h/2)/resY*720*DALScreenAdjust;
            handXPos = (hand1pos.x-w/2)/resX*1366;
        }
        else{
            handYPos = (hand1pos.y-h)/resY*720*DALScreenAdjust;
        }
        ofSetColor(205, 0, 40);
        if(w>0){
            ofRect(handXPos-1, handYPos-1, (w+2)/resX*1366, (h+2)/resY*720*DALScreenAdjust);
            ofRect(handXPos-2, handYPos-2, (w+4)/resX*1366, (h+4)/resY*720*DALScreenAdjust);
            ofRect(handXPos-3, handYPos-3, (w+6)/resX*1366, (h+6)/resY*720*DALScreenAdjust);
        }
        else{
            ofRect(handXPos+1, handYPos-1, (w-2)/resX*1366, (h+2)/resY*720*DALScreenAdjust);
            ofRect(handXPos+2, handYPos-2, (w-4)/resX*1366, (h+4)/resY*720*DALScreenAdjust);
            ofRect(handXPos+3, handYPos-3, (w-6)/resX*1366, (h+6)/resY*720*DALScreenAdjust);
        }
    }
    ofFill();
    
    ofSetColor(255,0,0); // red
    
    if(hand1pos.x != 0 && hand1pos.y != 0){
        ofRect((hand1pos.x-5)/resX*1366, (hand1pos.y-5)/resY*720*DALScreenAdjust, 10, 10);
        
    }
    if(hand2pos.x != 0 && hand2pos.y != 0){
        ofRect((hand2pos.x-5)/resX*1366, (hand2pos.y-5)/resY*720*DALScreenAdjust, 10, 10);
        
    }
    ofPopStyle();
}

void ofApp::playInteractions(){
    
    
    //   // Draw playback video
    //    if(MemoryPlayback.isLoaded())
    //    {
    //        ofPushStyle();
    //        ofFill();
    //        ofSetColor(255);
    //       ofRectangle recordedRect(ofRectangle(0,0, MemoryPlayback.getWidth(), MemoryPlayback.getHeight()));
    //        recordedRect.scaleTo(ofRectangle(0,0,1920,1080));
    //        MemoryPlayback.draw(recordedRect);
    //        ofPopStyle();
    //    }
}

//--------------------------------------------------------------
// Event Handlers
//--------------------------------------------------------------
void ofApp::userEvent(ofxOpenNIUserEvent & event){
    
}
//--------------------------------------------------------------
// Helper Functions
//--------------------------------------------------------------

ofPoint ofApp::coordinateProcessor(ofPoint coordinates)
{
    
    float x = coordinates.x;
    float y = coordinates.y;
    // Multiplies coordinates by the ratio between the max interaction size possible & the desired resoloution
    x = resX * (x / resizeMaxWidth);
    y = resY * (y / resizeMaxHeight);
    
    // Trim coordinates to ensure its possible to reach the top & left boundraies
    // x = x + trimX;
    // y = y + trimY;
    
    // Prevents coodinates from exceeding the bounds of the screen
    if(x > resX) x = resX;
    if(y > resY) y = resY;
    if(x < 0) x = 0;
    if(y < 0) y = 0;
    
    // Returns results as type ofPoint
    ofPoint result;
    result.set(x, y);
    return result;
}


//--------------------------------------------------------------
void ofApp::launchGUI() {
    
    // guiDepthMin = minDepthSetter;
    // guiDepthMax = maxDepthSetter;
    // guiPlayTime = absolutePlayTime;
    
    gui1 = new ofxUISuperCanvas("Dynamic Config.", 10,10,debugPanelW,debugPanelH, OFX_UI_FONT_SMALL);
    //string _label, float x, float y, float w, float h, int _size = OFX_UI_FONT_MEDIUM);
    
    // KINECT DEPTH MIN
    gui1->addIntSlider("Depth Min", 0, 4000, &nearThreshold)->setTriggerType(OFX_UI_TRIGGER_ALL);
    
    // KINECT DEPTH MAX
    gui1->addIntSlider("Depth Max", 500, 8000, &farThreshold)->setTriggerType(OFX_UI_TRIGGER_BEGIN|OFX_UI_TRIGGER_CHANGE|OFX_UI_TRIGGER_END);
    gui1->addSpacer();
    
    // KINECT Dimension Width
    gui1->addIntSlider("Dimension Width", 200, 4000, &resX)->setTriggerType(OFX_UI_TRIGGER_ALL);
    
    // KINECT Dimension Height
    gui1->addIntSlider("Dimension Height", 200, 4000, &resY)->setTriggerType(OFX_UI_TRIGGER_ALL);
    gui1->addSpacer();
   
    // Screensaver length
    gui1->addIntSlider("Screensaver Time", 2000, 30000, &screenEndTime)->setTriggerType(OFX_UI_TRIGGER_ALL);
    gui1->addSpacer();
    
    ofAddListener(gui1->newGUIEvent,this,&ofApp::guiEvent);
}

//--------------------------------------------------------------
void ofApp::guiEvent(ofxUIEventArgs &e) {
    string name = e.getName();
    int kind = e.getKind();
    
}

// Displays Debug Information
void ofApp::displayDebug(){
    
    //ofRectangle previewWindow(1760,960,160,120); // shows live feed
        
        
        ofPushStyle();
        ofSetColor(255);
    
    stringstream ss;
    ss << "video queue size: " << vidRecorder.getVideoQueueSize() << endl
    << "audio queue size: " << vidRecorder.getAudioQueueSize() << endl
    << "FPS: " << + ofGetFrameRate() << endl
    << "Width: " << w << endl
    << "Height: " << h << endl
    << "Interacting? " << (interacting?"Yes":"No") << endl
    << "Recording? " << (bRecording?"Yes":"No") << endl
    << "Hands Tracking: " << openNIDevice.getNumTrackedHands() << endl
    << "User is interacting: " << (openNIDevice.getNumTrackedUsers()>0?"Yes":"No") << endl;
    
    ofSetColor(0,0,0,100);
    ofRect(0, 0, 260, 75);
    ofSetColor(255, 255, 255);
    ofDrawBitmapString(ss.str(), 15, 420);
    
    ofPopStyle();
    
        if(bRecording){
            ofSetColor(255, 0, 0);
            ofCircle(ofGetWidth() - 20, 20, 5);
            ofSetColor(255, 255, 255);
        }
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){
    
    if (key == 'd') {
        debugMode = !debugMode;
        
        if(gui1 != NULL) {
            gui1->toggleVisible();
        }
        
        if (debugMode==true) {
            if (gui1==NULL) {
                //once only.
                launchGUI();
            }
            ofShowCursor();
        } else {
            ofHideCursor();
        }
    }
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){
    
}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){
    
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){
    
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){
    
}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){
    
}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){
    
}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){
    
}
