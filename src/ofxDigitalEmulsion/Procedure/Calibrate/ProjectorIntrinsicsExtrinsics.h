#pragma once

#include "../Base.h"
#include "ofxCvMin.h"
#include "ofxOsc.h"

namespace ofxDigitalEmulsion {
	namespace Procedure {
		namespace Calibrate {
			class ProjectorIntrinsicsExtrinsics : public Base {
			public:
				struct Correspondence {
					ofVec3f world;
					ofVec2f projector;
				};

				ProjectorIntrinsicsExtrinsics();
				string getTypeName() const override;
				Graph::PinSet getInputPins() override;
				ofxCvGui::PanelPtr getView() override;
				void update() override;

				void serialize(Json::Value &) override;
				void deserialize(const Json::Value &) override;
			protected:
				void populateInspector2(ofxCvGui::ElementGroupPtr) override;

				void addPoint(float x, float y, int projectorWidth, int projectorHeight);
				void calibrate();

				Graph::PinSet inputPins;

				ofxOscReceiver oscServer;

				vector<Correspondence> correspondences;

				ofSoundPlayer failSound, successSound;
				float lastSeenSuccess;
				float lastSeenFail;

				float error;
			};
		}
	}
}
