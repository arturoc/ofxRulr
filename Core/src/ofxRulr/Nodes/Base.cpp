#include "pch_RulrCore.h"
#include "Base.h"

#include "ofxRulr/Graph/Editor/NodeHost.h"
#include "../Exception.h"
#include "Graphics.h"

using namespace ofxCvGui;

namespace ofxRulr {
	namespace Nodes {
		//----------
		Base::Base() {
			this->initialized = false;
			this->lastFrameUpdate = 0;
			this->updateAllInputsFirst = true;
		}

		//----------
		Base::~Base() {
			if (this->initialized) {
				//pins will try to notify this node when connections are dropped, so drop the pins first
				for (auto pin : this->inputPins) {
					pin->resetConnection();
				}
				this->onDestroy.notifyListenersInReverse();
				this->initialized = false;
			}
		}

		//----------
		string Base::getTypeName() const {
			return "Node";
		}

		//----------
		void Base::init() {
			this->onInspect.addListener([this](ofxCvGui::InspectArguments & args) {
				this->populateInspector(args);
			}, 99999, this); // populate the instpector with this at the top. We call notify in reverse for inheritance

			//notify the subclasses to init
			this->onInit.notifyListeners();
			this->initialized = true;
		}

		//----------
		void Base::update() {
			auto currentFrameIndex = ofGetFrameNum() + 1; // otherwise confusions at 0th frame
			if (currentFrameIndex > this->lastFrameUpdate) {
				this->lastFrameUpdate = currentFrameIndex;
				if (this->updateAllInputsFirst) {
					for (auto inputPin : this->inputPins) {
						auto inputNode = inputPin->getConnectionUntyped();
						if (inputNode) {
							inputNode->update();
						}
					}
				}
				this->onUpdate.notifyListeners();
			}
		}

		//----------
		string Base::getName() const {
			if (this->name.empty()) {
				return this->getTypeName();
			} else {
				return this->name;
			}
		}

		//----------
		void Base::setName(const string name) {
			this->name = name;
		}

		//----------
		void Base::setNodeHost(Graph::Editor::NodeHost * nodeHost) {
			this->nodeHost = nodeHost;
		}

		//----------
		Graph::Editor::NodeHost * Base::getNodeHost() const {
			return this->nodeHost;
		}

		//----------
		shared_ptr<ofImage> Base::getIcon() {
			if (!this->icon) {
				this->icon = Graphics::X().getIcon(this->getTypeName());
			}
			return this->icon;
		}

		//----------
		const ofColor & Base::getColor() {
			if (!this->color) {
				this->color = make_shared<ofColor>(Graphics::X().getColor(this->getTypeName()));
			}
			return * this->color;
		}

		//----------
		void Base::setIcon(shared_ptr<ofImage> icon) {
			this->icon = icon;
		}

		//----------
		void Base::setColor(const ofColor & color) {
			this->color = make_shared<ofColor>(color);
		}

		//----------
		const Graph::PinSet & Base::getInputPins() const {
			return this->inputPins;
		}

		//----------
		void Base::populateInspector(ofxCvGui::InspectArguments &inspectArguments) {
			auto inspector = inspectArguments.inspector;
			
			auto nameWidget = Widgets::Title::make(this->getName(), ofxCvGui::Widgets::Title::Level::H1);
			auto nameWidgetWeak = weak_ptr<Element>(nameWidget);
			nameWidget->onDraw += [this](ofxCvGui::DrawArguments & args) {
				ofxAssets::image("ofxCvGui::edit").draw(ofRectangle(args.localBounds.width - 20, 5, 15, 15));
			};
			nameWidget->onMouseReleased += [this, nameWidgetWeak](ofxCvGui::MouseArguments & args) {
				auto nameWidget = nameWidgetWeak.lock();
				if (nameWidget) {
					auto result = ofSystemTextBoxDialog("Change name of [" + this->getTypeName() + "] node (" + this->getName() + ")");
					if (result != "") {
						this->setName(result);
						nameWidget->setCaption(result);
					}
				}
			};
			inspector->add(nameWidget);

			inspector->add(Widgets::Title::make(this->getTypeName(), ofxCvGui::Widgets::Title::Level::H3));

			inspector->add(Widgets::Button::make("Save Node...", [this] () {
				try {
					auto result = ofSystemSaveDialog(this->getDefaultFilename(), "Save node [" + this->getName() + "] as json");
					if (result.bSuccess) {
						this->save(result.getPath());
					}
				}
				RULR_CATCH_ALL_TO_ALERT
			}));

			inspector->add(Widgets::Button::make("Load Node...", [this] () {
				try {
					auto result = ofSystemLoadDialog("Load node [" + this->getName() + "] from json");
					if (result.bSuccess) {
						this->load(result.getPath());
					}
				}
				RULR_CATCH_ALL_TO_ALERT
			}));

			//pin status
			for (auto inputPin : this->getInputPins()) {
				inspector->add(Widgets::Indicator::make(inputPin->getName(), [inputPin]() {
					return (Widgets::Indicator::Status) inputPin->isConnected();
				}));
			}

			//node parameters
			inspector->add(Widgets::Spacer::make());
		}

		//----------
		void Base::throwIfMissingAnyConnection() const {
			const auto inputPins = this->getInputPins();
			for(auto & inputPin : inputPins) {
				if (!inputPin->isConnected()) {
					stringstream message;
					message << "Node [" << this->getTypeName() << "] is missing connection [" << inputPin->getName() << "]";
					throw(Exception(message.str()));
				}
			}
		}

		//----------
		void Base::addInput(shared_ptr<Graph::AbstractPin> pin) {
			//setup events to fire on this node for this pin
			auto pinWeak = weak_ptr<Graph::AbstractPin>(pin);
			pin->onNewConnectionUntyped += [this, pinWeak](shared_ptr<Base> &) {
				auto pin = pinWeak.lock();
				if (pin) {
					this->onConnect(pin);
				}
				this->onAnyInputConnectionChanged.notifyListeners();
			};
			pin->onDeleteConnectionUntyped += [this, pinWeak](shared_ptr<Base> &) {
				auto pin = pinWeak.lock();
				if (pin) {
					this->onDisconnect(pin);
				}
				this->onAnyInputConnectionChanged.notifyListeners();
			};

			this->inputPins.add(pin);
		}

		//----------
		void Base::removeInput(shared_ptr<Graph::AbstractPin> pin) {
			this->inputPins.remove(pin);
		}

		//----------
		void Base::clearInputs() {
			this->inputPins.clear();
		}

		//----------
		void Base::setUpdateAllInputsFirst(bool updateAllInputsFirst) {
			this->updateAllInputsFirst = updateAllInputsFirst;
		}

		//----------
		bool Base::getUpdateAllInputsFirst() const {
			return this->updateAllInputsFirst;
		}
	}
}