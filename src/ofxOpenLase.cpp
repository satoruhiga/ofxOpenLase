#include "ofxOpenLase.h"

#define NOT_IMPL ofLogError("ofxOpenLaseRenderer", "not implemented: " + string(__PRETTY_FUNCTION__)), throw "not implemented";

static ofPtr<ofxOpenLaseRenderer> renderer;
static ofPtr<ofRendererCollection> rendererCollection;

ofxOpenLaseRenderer::ofxOpenLaseRenderer()
{
	OLRenderParams params;
	memset(&params, 0, sizeof params);
	params.rate = 48000;
	params.on_speed = 2.0/100.0;
	params.off_speed = 2.0/30.0;
	params.start_wait = 4;
	params.start_dwell = 3;
	params.curve_dwell = 0;
	params.corner_dwell = 4;
	params.curve_angle = cosf(30.0*(M_PI/180.0)); // 30 deg
	params.end_dwell = 2;
	params.end_wait = 5;
	params.snap = 1/100000.0;
	params.render_flags = RENDER_GRAYSCALE;
	
	if(olInit(32, 30000) < 0)
		ofExit(-1);
	
	olSetRenderParams(&params);
	
	ofAddListener(ofEvents.draw, this, &ofxOpenLaseRenderer::onDraw);
}

ofxOpenLaseRenderer::~ofxOpenLaseRenderer()
{
	try
	{
		ofRemoveListener(ofEvents.draw, this, &ofxOpenLaseRenderer::onDraw);
	} catch (...) {}
	
	olShutdown();
}

void ofxOpenLaseRenderer::setCircleResolution(int res)
{
	if((int)circlePolyline.size()!=res+1){
		circlePolyline.clear();
		circlePolyline.arc(0,0,0,1,1,0,360,res);
	}
}

void ofxOpenLaseRenderer::update()
{
}

void ofxOpenLaseRenderer::draw(ofPolyline& poly)
{
	draw(poly.getVertices(), OF_PRIMITIVE_LINE_STRIP);
}

void ofxOpenLaseRenderer::draw(ofPath& shape)
{
	vector<ofPolyline> outlines;
	for (int i = 0; i < outlines.size(); i++)
	{
		draw(outlines[i]);
	}
}

void ofxOpenLaseRenderer::draw(ofMesh& vertexData)
{
	int mode = vertexData.getMode() != OF_PRIMITIVE_POINTS ? OL_LINESTRIP : OL_POINTS;
	
	ofIndexType num_index = vertexData.getNumIndices();
	bool has_color = vertexData.getNumColors() > 0;
	
	olBegin(mode);
	if (num_index > 0)
	{
		if (has_color)
		{
			for (int i = 0; i < num_index; i++)
			{
				ofIndexType idx = vertexData.getIndex(i);
				ofColor c = vertexData.getColor(idx) * 255;
				ofVec3f v = vertexData.getVertex(idx);
				olVertex3(v.x, v.y, v.z, c.getHex());
			}
		}
		else
		{
			for (int i = 0; i < num_index; i++)
			{
				ofIndexType idx = vertexData.getIndex(i);
				ofVec3f v = vertexData.getVertex(idx);
				olVertex3(v.x, v.y, v.z, currentColorHex);
			}
		}
	}
	else
	{
		if (has_color)
		{
			for (int i = 0; i < vertexData.getNumVertices(); i++)
			{
				ofColor c = vertexData.getColor(i) * 255;
				ofVec3f v = vertexData.getVertex(i);
				olVertex3(v.x, v.y, v.z, c.getHex());
			}
		}
		else
		{
			for (int i = 0; i < vertexData.getNumVertices(); i++)
			{
				ofVec3f v = vertexData.getVertex(i);
				olVertex3(v.x, v.y, v.z, currentColorHex);
			}
		}
	}
	olEnd();
}

void ofxOpenLaseRenderer::draw(ofMesh& vertexData, ofPolyRenderMode renderType)
{
	ofPrimitiveMode mode = OF_PRIMITIVE_LINE_STRIP;
	if (renderType == OF_MESH_POINTS) mode = OF_PRIMITIVE_POINTS;
	vertexData.setMode(mode);
	draw(vertexData);
}

void ofxOpenLaseRenderer::draw(vector<ofPoint>& vertexData, ofPrimitiveMode drawMode)
{
	ofMesh mesh;
	for (int i = 0; i < vertexData.size(); i++)
	{
		mesh.addColor(currentFloatColor);
		mesh.addVertex(vertexData[i]);
	}
	
	if (drawMode == OF_PRIMITIVE_TRIANGLES
		|| drawMode == OF_PRIMITIVE_TRIANGLE_STRIP
		|| drawMode == OF_PRIMITIVE_TRIANGLE_FAN)
	{
		drawMode = OF_PRIMITIVE_TRIANGLE_STRIP;
	}
	
	mesh.setMode(drawMode);
	
	draw(mesh);
}

void ofxOpenLaseRenderer::drawLine(float x1, float y1, float z1, float x2, float y2, float z2)
{
	ofMesh mesh;
	mesh.addColor(currentFloatColor);
	mesh.addVertex(ofVec3f(x1,y1,z1));
	mesh.addColor(currentFloatColor);
	mesh.addVertex(ofVec3f(x2,y2,z2));
	draw(mesh);
}

void ofxOpenLaseRenderer::drawRectangle(float x, float y, float z, float w, float h)
{
	ofMesh mesh;
	if (rectMode == OF_RECTMODE_CORNER){
		mesh.addColor(currentFloatColor);
		mesh.addVertex(ofVec3f(x,y,z));
		mesh.addColor(currentFloatColor);
		mesh.addVertex(ofVec3f(x+w, y, z));
		mesh.addColor(currentFloatColor);
		mesh.addVertex(ofVec3f(x+w, y+h, z));
		mesh.addColor(currentFloatColor);
		mesh.addVertex(ofVec3f(x, y+h, z));
	}else{
		mesh.addColor(currentFloatColor);
		mesh.addVertex(ofVec3f(x-w/2.0f, y-h/2.0f, z));
		mesh.addColor(currentFloatColor);
		mesh.addVertex(ofVec3f(x+w/2.0f, y-h/2.0f, z));
		mesh.addColor(currentFloatColor);
		mesh.addVertex(ofVec3f(x+w/2.0f, y+h/2.0f, z));
		mesh.addColor(currentFloatColor);
		mesh.addVertex(ofVec3f(x-w/2.0f, y+h/2.0f, z));
	}
	draw(mesh);
}

void ofxOpenLaseRenderer::drawTriangle(float x1, float y1, float z1, float x2, float y2, float z2, float x3, float y3, float z3)
{
	ofMesh mesh;
	mesh.addColor(currentFloatColor);
	mesh.addVertex(ofVec3f(x1, y1, z1));
	mesh.addColor(currentFloatColor);
	mesh.addVertex(ofVec3f(x2, y2, z2));
	mesh.addColor(currentFloatColor);
	mesh.addVertex(ofVec3f(x3, y3, z3));
	draw(mesh);
}

void ofxOpenLaseRenderer::drawCircle(float x, float y, float z, float radius)
{
	ofMesh mesh;
	
	for (int i = 0; i < circlePolyline.size(); i++)
	{
		ofVec3f &v = circlePolyline[i];
		mesh.addColor(currentFloatColor);
		mesh.addVertex(ofVec3f(v.x * radius + x, v.y * radius + y, z));
	}
	
	draw(mesh);
}

void ofxOpenLaseRenderer::drawEllipse(float x, float y, float z, float width, float height)
{
	ofMesh mesh;
	
	float radiusX = width*0.5;
	float radiusY = height*0.5;
	for (int i = 0; i < circlePolyline.size(); i++)
	{
		ofVec3f &v = circlePolyline[i];
		mesh.addColor(currentFloatColor);
		mesh.addVertex(ofVec3f(v.x * radiusX + x, v.y * radiusY + y, z));
	}

	draw(mesh);
}

void ofxOpenLaseRenderer::drawString(string text, float x, float y, float z, ofDrawBitmapMode mode)
{
	NOT_IMPL;
}

#pragma mark -

void ofxOpenLaseRenderer::setupScreenPerspective(float width, float height, ofOrientation orientation, bool vFlip, float fov, float nearDist, float farDist)
{
	if(width == 0) width = ofGetWidth();
	if(height == 0) height = ofGetHeight();
	
	float viewW = ofGetViewportWidth();
	float viewH = ofGetViewportHeight();
	
	float eyeX = viewW / 2;
	float eyeY = viewH / 2;
	float halfFov = PI * fov / 360;
	float theTan = tanf(halfFov);
	float dist = eyeY / theTan;
	float aspect = (float) viewW / viewH;
	
	if(nearDist == 0) nearDist = dist / 10.0f;
	if(farDist == 0) farDist = dist * 10.0f;
	
	ofMatrix4x4 projection, modelview;
	projection.makePerspectiveMatrix(fov, aspect, nearDist, farDist);
	modelview.makeLookAtViewMatrix(ofVec3f(eyeX, eyeY, dist), ofVec3f(eyeX, eyeY, 0), ofVec3f(0, 1, 0));
	
	if(vFlip){
		modelview.glScale(1, -1, 1);
		modelview.glTranslate(0, -height, 0);
	}
	
	modelviewProjectionMatrix = projection;
	modelviewProjectionMatrix.preMult(modelview);
	
	olLoadIdentity3();
	olMultMatrix3(modelviewProjectionMatrix.getPtr());
}

void ofxOpenLaseRenderer::setupScreenOrtho(float width, float height, ofOrientation orientation, bool vFlip, float nearDist, float farDist)
{
	NOT_IMPL;
}

#pragma mark -

void ofxOpenLaseRenderer::pushMatrix()
{
	olPushMatrix3();
}

void ofxOpenLaseRenderer::popMatrix()
{
	olPopMatrix3();
}

void ofxOpenLaseRenderer::translate(float x, float y, float z)
{
	olTranslate3(x, y, z);
}

void ofxOpenLaseRenderer::translate(const ofPoint & p)
{
	olTranslate3(p.x, p.y, p.z);
}

void ofxOpenLaseRenderer::scale(float xAmnt, float yAmnt, float zAmnt)
{
	olScale3(xAmnt, yAmnt, zAmnt);
}

void ofxOpenLaseRenderer::rotate(float degrees, float vecX, float vecY, float vecZ)
{
	ofMatrix4x4 matrix;
	matrix.glRotate(degrees, vecX, vecY, vecZ);
	olMultMatrix3(matrix.getPtr());
}

void ofxOpenLaseRenderer::rotateX(float degrees)
{
	olRotate3X(ofDegToRad(degrees));
}

void ofxOpenLaseRenderer::rotateY(float degrees)
{
	olRotate3Y(ofDegToRad(degrees));
}

void ofxOpenLaseRenderer::rotateZ(float degrees)
{
	olRotate3Z(ofDegToRad(degrees));
}

void ofxOpenLaseRenderer::rotate(float degrees)
{
	olRotate3Z(ofDegToRad(degrees));
}

void ofxOpenLaseRenderer::setColor(const ofColor & color)
{
	currentColor.set(color);
	currentColorHex = currentColor.getHex();
	
	currentFloatColor = currentColor;
	currentFloatColor /= 255.;
}

#pragma mark -

void ofxOpenLaseRenderer::onDraw(ofEventArgs&)
{
	float ftime = olRenderFrame(60);
}

#pragma mark -

void ofxOpenLaseEnable()
{
	if (!rendererCollection)
	{
		renderer = ofPtr<ofxOpenLaseRenderer>(new ofxOpenLaseRenderer);
		
		rendererCollection = ofPtr<ofRendererCollection>(new ofRendererCollection);
		rendererCollection->renderers.push_back(ofGetCurrentRenderer());
		rendererCollection->renderers.push_back(renderer);
	}
	
	ofSetCurrentRenderer(rendererCollection);
}

void ofxOpenLaseDisable()
{
	ofSetCurrentRenderer(ofGetGLRenderer());
	renderer.reset();
}
