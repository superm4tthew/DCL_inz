#ifndef ExtractSURF_HPP_
#define ExtractSURF_HPP_

#include <ctime>
#include "Component_Aux.hpp"
#include "Component.hpp"
#include "Panel_Empty.hpp"
#include "DataStream.hpp"
#include <opencv2/highgui/highgui.hpp>

#include "Property.hpp"

#include <cv.h>

namespace Processors {
namespace ExtractSURF {

using namespace cv;

class ExtractSURF: public Base::Component {
public:
	//Constructor. 
	ExtractSURF(const std::string & name = "");
	
	//Destructor
	virtual ~ExtractSURF();
protected:
	//Connects source to given device. 
	bool onInit();

	//Disconnect source from device, closes streams, etc. 
	bool onFinish();
	
	//Retrieves data from device. 
	bool onStep();

	//Start component 
	bool onStart();
	
	//Stop component 
	bool onStop();

	void onNewImage();
	
	Base::EventHandler <ExtractSURF> h_onNewImage;
	Base::Event*NewImage;
	Base::Event*Computed;

	Base::DataStreamIn  <Mat> in_img;

	Base::DataStreamOut <std::vector<cv::KeyPoint> > out_keypoints;
	Base::DataStreamOut <Mat> out_descriptors;
	Base::DataStreamOut <Mat> out_keypoint_image;

private:
		
	SurfFeatureDetector *SURF_Detector;
	
	Base::Property<float>  SURFthreshold;

};

}//: namespace ExtractSURF
}//: namespace Processors

//Register processor component.
REGISTER_PROCESSOR_COMPONENT("ExtractSURF", Processors::ExtractSURF::ExtractSURF, Common::Panel_Empty)

#endif /* ExtractSURF_HPP_ */
