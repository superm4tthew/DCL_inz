#include <memory>
#include <string>
#include "ExtractSURF.hpp"
#include "Common/Logger.hpp"
#include <iostream>
#include <sstream>


namespace Processors {
namespace ExtractSURF {

ExtractSURF::ExtractSURF(const std::string & name) :
	Base::Component(name), SURFthreshold("threshold", 2500.) {
	LOG(LTRACE) << "Hello ExtractSURF\n";
	registerProperty( SURFthreshold);
}

ExtractSURF::~ExtractSURF() {
	LOG(LTRACE) << "Good bye ExtractSURF\n";
}

bool ExtractSURF::onInit() {
	//LOG(LTRACE) << "ExtractSURF::initialize\n";
	h_onNewImage.setup(this, &ExtractSURF::onNewImage);
	registerHandler("onNewImage", &h_onNewImage);
	NewImage = registerEvent("NewImage");
	Computed = registerEvent("Computed");
	registerStream("in_img", &in_img);
	registerStream("out_descriptors", &out_descriptors);
	registerStream("out_keypoint_image", &out_keypoint_image);
	registerStream("out_keypoints", &out_keypoints);
	
	return true;
}

bool ExtractSURF::onFinish() {
	return true;
}

bool ExtractSURF::onStep() {
	return true;
}

bool ExtractSURF::onStop() {
	return true;
}

bool ExtractSURF::onStart() {
	SURF_Detector = new cv::SurfFeatureDetector(SURFthreshold);		
	return true;
}

void ExtractSURF::onNewImage() {
	try {
		Mat img = in_img.read();
		Mat out_img = img.clone();
		
		SurfDescriptorExtractor SURF_Descriptor_Extractor;
		Mat descriptors;
		std::vector<cv::KeyPoint> keypoints;
		
		SURF_Detector->detect(img, keypoints);
		SURF_Descriptor_Extractor.compute(img, keypoints, descriptors);
		
		// Draw the keypoints with scale and orientation information
		cv::drawKeypoints(img, keypoints, out_img, Scalar(255, 255, 255), DrawMatchesFlags::DRAW_RICH_KEYPOINTS);

		out_keypoint_image.write(out_img);
		out_descriptors.write(descriptors);
		out_keypoints.write(keypoints);
		
		Computed->raise();

	} catch (...) {
	//	LOG(LERROR) << "FeatureExtractor::onNewImage failed\n";
	}

}

}//: namespace ExtractSURF
}//: namespace Processors
