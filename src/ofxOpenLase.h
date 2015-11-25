#pragma once

#include "ofMain.h"

extern "C"
{
#include "libol.h"
#include <jack/jack.h>
}

#define OFX_OPENLASE_BEGIN_NAMESPACE namespace ofx { namespace OpenLase {
#define OFX_OPENLASE_END_NAMESPACE } }

OFX_OPENLASE_BEGIN_NAMESPACE

class ConnectionManager
{
public:
	
	ConnectionManager()
		: client(nullptr)
	{}
	
	virtual ~ConnectionManager()
	{
		if (client != nullptr)
		{
			jack_client_close(client);
			client = nullptr;
		}
	}
	
	bool setup()
	{
		jack_status_t status;
		jack_client_t* c = jack_client_open("ConnectionManager", JackNullOption, &status);
		
		if (c == nullptr)
			return false;
		
		client = c;
		
		return true;
	}
	
	void dumpAllPortName() const
	{
		const char** ports = jack_get_ports(client, NULL, NULL, 0);
		const char** it = ports;
		
		cout << "===" << endl;
		
		while (*it != NULL)
		{
			cout << *it << endl;
			it++;
		}
		
		cout << "===" << endl;
		
		free(ports);
	}
	
	bool connect(const string& from, const string& to)
	{
		jack_connect(client, from.c_str(), to.c_str());
	}
	
protected:
	
	jack_client_t* client;
};

//

class Renderer
{
public:
	
	Renderer()
		: current_color_hex(0xFFFFFFFF)
		, draw_opengl(true)
		, current_camera(nullptr)
	{}
	
	virtual ~Renderer()
	{
		olShutdown();
	}
	
	bool setup(int rate = 48000, bool draw_opengl = true)
	{
		this->draw_opengl = draw_opengl;
		
		memset(&params, 0, sizeof params);
		
		params.rate = rate;
		
		params.on_speed = 0.1;
		params.off_speed = 0.1;
		
		params.start_wait = 1;
		params.start_dwell = 3;
		
		params.curve_dwell = 0;
		params.corner_dwell = 8;
		
		params.curve_angle = cosf(30.0*(M_PI/180.0)); // 30 deg
		
		params.end_dwell = 3;
		params.end_wait = 7;
		
		params.snap = 1/100000.0;
		params.max_framelen = params.rate / 30;
		params.flatness = 0.00001;
		params.render_flags = RENDER_GRAYSCALE;
		
		if(olInit(32, 300000) < 0)
		{
			ofSystemAlertDialog("jack server not running?");
			ofExit(-1);
		}
		
		olSetRenderParams(&params);
		
		return true;
	}
	
	void begin()
	{
		ofMatrix4x4 projection;
		ofMatrix4x4 modelview;
		ofMatrix4x4 modelview_projection;
		
		glGetFloatv(GL_MODELVIEW_MATRIX, modelview.getPtr());
		glGetFloatv(GL_PROJECTION_MATRIX, projection.getPtr());
		
		modelview_projection = projection;
		modelview_projection.preMult(modelview);
		
		olLoadIdentity3();
		olMultMatrix3(modelview_projection.getPtr());
		
		while (matrix_stack.size())
			matrix_stack.pop();
		
		current_modelview.makeIdentityMatrix();
		matrix_stack.push(current_modelview);
	}
	
	void begin(ofCamera& cam)
	{
		cam.begin();
		cam.end();
		
		ofMatrix4x4 projection = cam.getProjectionMatrix();
		ofMatrix4x4 modelview = cam.getModelViewMatrix();
		ofMatrix4x4 modelview_projection;
		
		modelview_projection = projection;
		modelview_projection.preMult(modelview);
		
		olLoadIdentity3();
		olMultMatrix3(modelview_projection.getPtr());
		
		while (matrix_stack.size())
			matrix_stack.pop();
		
		current_modelview.makeIdentityMatrix();
		matrix_stack.push(current_modelview);
		
		current_camera = &cam;
		current_camera->begin();
	}
	
	void end()
	{
		if (current_camera)
		{
			current_camera->end();
			current_camera = nullptr;
		}
	}
	
	void flush()
	{
		float ftime = olRenderFrame(120);
	}
	
	void setCamera()
	{
		olLoadIdentity3();
	}
	
	//
	
	void pushMatrix()
	{
		matrix_stack.push(current_modelview);
	}
	
	void popMatrix()
	{
		current_modelview = matrix_stack.top();
		matrix_stack.pop();
	}
	
	void translate(float x, float y, float z)
	{
		current_modelview.glTranslate(x, y, z);
	}
	
	void rotate(float angle, float x, float y, float z)
	{
		current_modelview.glRotate(angle, x, y, z);
	}
	
	void scale(float x, float y, float z)
	{
		current_modelview.glScale(x, y, z);
	}
	
	//
	
	void color(const ofColor& color)
	{
		current_color_hex = color.getHex();
	}
	
	//
	
	void draw(const ofMesh& vertexData) const
	{
		ofPrimitiveMode prim_mode = vertexData.getMode();
		int mode = prim_mode != OF_PRIMITIVE_POINTS ? OL_LINESTRIP : OL_POINTS;
		
		{
			begin_ol_modelview();
			
			if (mode == OL_LINESTRIP)
			{
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
							const ofColor& c = vertexData.getColor(idx) * 255;
							const ofVec3f& v = vertexData.getVertex(idx);
							olVertex3(v.x, v.y, v.z, c.getHex());
						}
					}
					else
					{
						for (int i = 0; i < num_index; i++)
						{
							ofIndexType idx = vertexData.getIndex(i);
							const ofVec3f& v = vertexData.getVertex(idx);
							olVertex3(v.x, v.y, v.z, current_color_hex);
						}
					}
				}
				else
				{
					if (has_color)
					{
						for (int i = 0; i < vertexData.getNumVertices(); i++)
						{
							const ofColor& c = vertexData.getColor(i) * 255;
							const ofVec3f& v = vertexData.getVertex(i);
							olVertex3(v.x, v.y, v.z, c.getHex());
						}
					}
					else
					{
						for (int i = 0; i < vertexData.getNumVertices(); i++)
						{
							const ofVec3f& v = vertexData.getVertex(i);
							olVertex3(v.x, v.y, v.z, current_color_hex);
						}
					}
				}
				olEnd();
			}
			else if (mode == OL_POINTS)
			{
				bool has_color = vertexData.getNumColors() > 0;
				
				olBegin(mode);
				
				if (has_color)
				{
					for (int i = 0; i < vertexData.getNumVertices(); i++)
					{
						const ofColor& c = vertexData.getColor(i) * 255;
						const ofVec3f& v = vertexData.getVertex(i);
						olVertex3(v.x, v.y, v.z, c.getHex());
					}
				}
				else
				{
					for (int i = 0; i < vertexData.getNumVertices(); i++)
					{
						const ofVec3f& v = vertexData.getVertex(i);
						olVertex3(v.x, v.y, v.z, current_color_hex);
					}
				}
				
				olEnd();
			}
			
			end_ol_modelview();
		}
		
		if (draw_opengl)
		{
			begin_of_modelview();
			
			if (prim_mode != OF_PRIMITIVE_POINTS)
				vertexData.drawWireframe();
			else
				vertexData.drawVertices();
			
			end_of_modelview();
		}
	}
	
	void draw(const ofPolyline& vertexData) const
	{
		int mode = OL_LINESTRIP;
		
		{
			begin_ol_modelview();
			
			if (mode == OL_LINESTRIP)
			{
				const int size = vertexData.size();
				
				olBegin(mode);
				
				for (int i = 0; i < size; i++)
				{
					const ofVec3f& v = vertexData[i];
					olVertex3(v.x, v.y, v.z, current_color_hex);
				}
				
				olEnd();
			}
			
			end_ol_modelview();
		}
		
		if (draw_opengl)
		{
			begin_of_modelview();
			
			vertexData.draw();
			
			end_of_modelview();
		}
	}
	
	void setOnOffSpeedParam(float on, float off = -1)
	{
		if (off < 0) off = on;
		
		params.on_speed = on;
		params.off_speed = off;
		
		olSetRenderParams(&params);
	}
	
	void setStartParam(int wait, int dwell)
	{
		params.start_wait = wait;
		params.start_dwell = dwell;
		
		olSetRenderParams(&params);
	}
	
	void setCurveParam(int wait, int dwell, float angle = 30)
	{
		params.curve_dwell = wait;
		params.corner_dwell = dwell;
		params.curve_angle = cosf(angle*(M_PI/180.0)); // 30 deg
		
		olSetRenderParams(&params);
	}
	
	void setEndParam(int wait, int dwell)
	{
		params.end_wait = wait;
		params.end_dwell = dwell;
		
		olSetRenderParams(&params);
	}
	
	void setSnap(float v)
	{
		params.snap = v;
		
		olSetRenderParams(&params);
	}
	
	void setMaxFrameLen(int v)
	{
		params.max_framelen = v;
		
		olSetRenderParams(&params);
	}
	
	void setFlatness(float v)
	{
		params.flatness = v;
		
		olSetRenderParams(&params);
	}
	
protected:
	
	OLRenderParams params;
	
	uint32_t current_color_hex;
	
	mutable ofMatrix4x4 current_modelview;
	stack<ofMatrix4x4> matrix_stack;
	
	ofCamera* current_camera;
	
	bool draw_opengl;
	
	void begin_ol_modelview() const
	{
		olPushMatrix3();
		olMultMatrix3(current_modelview.getPtr());
	}
	
	void end_ol_modelview() const
	{
		olPopMatrix3();
	}
	
	void begin_of_modelview() const
	{
		ofPushMatrix();
		ofMultMatrix(current_modelview);
	}
	
	void end_of_modelview() const
	{
		ofPopMatrix();
	}
};


OFX_OPENLASE_END_NAMESPACE

namespace ofxOpenLase = ofx::OpenLase;