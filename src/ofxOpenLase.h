#pragma once

#include "ofMain.h"

extern "C"
{
#include "libol.h"
}

class ofxOpenLaseRenderer : public ofBaseRenderer
{
public:
	
	ofxOpenLaseRenderer();
	~ofxOpenLaseRenderer();
	
	string getType() { return "openlase"; };
	
	void update();
	
	void draw(ofPolyline & poly);
	void draw(ofPath & shape);
	void draw(ofMesh & vertexData);
	void draw(ofMesh & vertexData, ofPolyRenderMode renderType);
	void draw(vector<ofPoint> & vertexData, ofPrimitiveMode drawMode);
	void draw(ofImage & image, float x, float y, float z, float w, float h) {}
	void draw(ofFloatImage & image, float x, float y, float z, float w, float h) {}
	void draw(ofShortImage & image, float x, float y, float z, float w, float h) {}
	
	void pushView() {}
	void popView() {}
	
	void viewport(ofRectangle viewport) { currentViewport = viewport; }
	void viewport(float x = 0, float y = 0, float width = 0, float height = 0, bool invertY = true) { currentViewport = ofRectangle(x, y, width, height); }
	ofRectangle getCurrentViewport(){ return currentViewport; }
	int getViewportWidth(){ return currentViewport.width; }
	int getViewportHeight(){return currentViewport.height; }

	void setupScreenPerspective(float width = 0, float height = 0, ofOrientation orientation=OF_ORIENTATION_UNKNOWN, bool vFlip = true, float fov = 60, float nearDist = 0, float farDist = 0);
	void setupScreenOrtho(float width = 0, float height = 0, ofOrientation orientation=OF_ORIENTATION_UNKNOWN, bool vFlip = true, float nearDist = -1, float farDist = 1);
	
	void setCoordHandedness(ofHandednessType handedness){};
	ofHandednessType getCoordHandedness(){return OF_RIGHT_HANDED;};
	
	void pushMatrix();
	void popMatrix();
	void translate(float x, float y, float z = 0);
	void translate(const ofPoint & p);
	void scale(float xAmnt, float yAmnt, float zAmnt = 1);
	void rotate(float degrees, float vecX, float vecY, float vecZ);
	void rotateX(float degrees);
	void rotateY(float degrees);
	void rotateZ(float degrees);
	void rotate(float degrees);
	
	void setupGraphicDefaults() { ofSetCoordHandedness(OF_RIGHT_HANDED); }
	void setupScreen() { setupScreenPerspective(); }
	
	void setRectMode(ofRectMode mode) { rectMode = mode; }
	ofRectMode getRectMode() { return rectMode; }
	
	void setFillMode(ofFillFlag fill) { fillFlag = fill; }
	ofFillFlag getFillMode() { return fillFlag; }
	void setLineWidth(float lineWidth) {}
	void setBlendMode(ofBlendMode blendMode) {}
	void setLineSmoothing(bool smooth) {}
	void setCircleResolution(int res);
	void enablePointSprites() {}
	void disablePointSprites() {}
	
	void setColor(int r, int g, int b) { setColor(ofColor(r, g, b, 255)); }
	void setColor(int r, int g, int b, int a) { setColor(ofColor(r, g, b, a)); }
	void setColor(const ofColor & color);
	void setColor(const ofColor & color, int _a) { setColor(color.r, color.g, color.b, _a); }
	void setColor(int gray) { setColor(gray, gray, gray, 255); }
	void setHexColor(int hexColor) { setColor(ofColor::fromHex(hexColor)); }
	
	ofFloatColor& getBgColor() {}
	bool bClearBg() { return true; }
	void background(const ofColor & c) {}
	void background(float brightness) {}
	void background(int hexColor, float _a=255.0f) {}
	void background(int r, int g, int b, int a=255) {}
	
	void setBackgroundAuto(bool bManual) {}
	
	void clear(float r, float g, float b, float a) {}
	void clear(float brightness, float a) {}
	void clearAlpha() {}
	
	// drawing
	void drawLine(float x1, float y1, float z1, float x2, float y2, float z2);
	void drawRectangle(float x, float y, float z, float w, float h);
	void drawTriangle(float x1, float y1, float z1, float x2, float y2, float z2, float x3, float y3, float z3);
	void drawCircle(float x, float y, float z, float radius);
	void drawEllipse(float x, float y, float z, float width, float height);
	void drawString(string text, float x, float y, float z, ofDrawBitmapMode mode);
	
	// returns true if the renderer can render curves without decomposing them
	bool rendersPathPrimitives() { return false; }
	
	void onDraw(ofEventArgs&);

protected:
	
	ofRectMode rectMode;
	ofFillFlag fillFlag;
	
	ofRectangle currentViewport;
	ofColor currentColor;
	ofFloatColor currentFloatColor;
	uint32_t currentColorHex;
	
	ofMatrix4x4 modelviewProjectionMatrix;
	ofPolyline circlePolyline;
	
};

void ofxOpenLaseEnable();
void ofxOpenLaseDisable();
