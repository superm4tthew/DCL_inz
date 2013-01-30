#ifndef ExtractSIFT_HPP_
#define ExtractSIFT_HPP_

#include <ctime>
#include "Component_Aux.hpp"
#include "Component.hpp"
#include "Panel_Empty.hpp"
#include "DataStream.hpp"
#include <opencv2/highgui/highgui.hpp>

#include "Property.hpp"

#include <cv.h>

namespace Processors {
namespace ExtractSIFT {

using namespace cv;

class ExtractSIFT: public Base::Component {
public:
	//Constructor. 
	ExtractSIFT(const std::string & name = "");
	
	//Destructor
	virtual ~ExtractSIFT();
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

	Base::EventHandler <ExtractSIFT> h_onNewImage;
	Base::Event*NewImage;
	Base::Event*Computed;

	Base::DataStreamIn  <Mat> in_img;
	
	Base::DataStreamOut <std::vector<cv::KeyPoint> > out_keypoints;
	Base::DataStreamOut <Mat> out_descriptors;
	Base::DataStreamOut <Mat> out_keypoint_image;

private:
		
	SiftFeatureDetector *SIFT_Detector;
	
	Base::Property<float>  SIFTFeatureThreshold;
	Base::Property<float>  SIFTLinesThreshold;
};

}//: namespace ExtractSIFT
}//: namespace Processors

//Register processor component.
REGISTER_PROCESSOR_COMPONENT("ExtractSIFT", Processors::ExtractSIFT::ExtractSIFT, Common::Panel_Empty)

#endif /* ExtractSIFT_HPP_ */
