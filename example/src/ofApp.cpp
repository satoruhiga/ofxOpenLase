#include "ofApp.h"

#include "ofxOpenLase.h"

ofxOpenLase::Renderer lase;
ofxOpenLase::ConnectionManager conn;

ofMesh mesh;

//--------------------------------------------------------------
void ofApp::setup(){
	ofSetFrameRate(60);
	ofSetVerticalSync(true);
	ofBackground(0);
	
	lase.setup();
	
	// preview utils -> ofxOpenLase/bin/simulator
	conn.setup();
	conn.connect("libol:out_x", "simulator:in_x");
	conn.connect("libol:out_y", "simulator:in_y");
	conn.connect("libol:out_g", "simulator:in_g");
	
	mesh = ofBoxPrimitive(100, 100, 100, 1, 1, 1).getMesh();
}

//--------------------------------------------------------------
void ofApp::update(){

}

//--------------------------------------------------------------
void ofApp::draw(){
	lase.begin();
	
	lase.translate(ofGetWidth() * 0.5, ofGetHeight() * 0.5, 0);
	lase.rotate(ofGetElapsedTimef() * 20, 0, 1, 0);
	
	lase.rotate(ofGetMouseX(), 1, 0, 0);
	lase.rotate(ofGetMouseY(), 0, 1, 0);
	
	float s = ofGetMousePressed() ? 1 : 2;
	lase.scale(s, s, s);
	
	lase.pushMatrix();
	lase.translate(200, 0, 0);
	lase.draw(mesh);
	lase.popMatrix();
	
	lase.draw(mesh);
	
	lase.end();
	
	lase.flush();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

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
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

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
