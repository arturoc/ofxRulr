#pragma once

#include "../Nodes/Base.h"

#include "ofxCvGui/Panels/World.h"

namespace ofxRulr {
	namespace Graph {
		class Summary : public Nodes::Base {
		public:
			Summary(const Utils::Set<Nodes::Base> & world);
			void init();
			string getTypeName() const override;
			ofxCvGui::PanelPtr getView() override;
		protected:
			void serialize(Json::Value &);
			void deserialize(const Json::Value &);
			void populateInspector(ofxCvGui::ElementGroupPtr);
			void callbackShowCursor(bool &);

			void drawGrid();

			shared_ptr<ofxCvGui::Panels::World> view;

			ofParameter<bool> showCursor;
			ofParameter<bool> showGrid;
			ofParameter<ofVec3f> roomMinimum;
			ofParameter<ofVec3f> roomMaximum;
			ofLight light;

			const Utils::Set<Nodes::Base> & world;

			ofImage * grid;
		};
	}
}