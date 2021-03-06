#include <memory>
#include <string>
#include "ExtractSIFT.hpp"
#include "Common/Logger.hpp"
#include <iostream>
#include <sstream>
#include <string>

namespace Processors {
namespace ExtractSIFT {

ExtractSIFT::ExtractSIFT(const std::string & name) :
	Base::Component(name), SIFTFeatureThreshold("FeaturesThreshold", 0.03), SIFTLinesThreshold("LinesThreshold", 10.) {
	registerProperty( SIFTFeatureThreshold);
	registerProperty( SIFTLinesThreshold);
}

ExtractSIFT::~ExtractSIFT() {
}

bool ExtractSIFT::onInit() {
	//std::cout<<"SIFT onInit"<<std::endl;
	//LOG(LTRACE) << "ExtractSIFT::initialize\n";
	h_onNewImage.setup(this, &ExtractSIFT::onNewImage);
	registerHandler("onNewImage", &h_onNewImage);
	NewImage = registerEvent("NewImage");
	Computed = registerEvent("Computed");
	registerStream("in_img", &in_img);
	registerStream("out_descriptors", &out_descriptors);
	registerStream("out_keypoint_image", &out_keypoint_image);
	registerStream("out_keypoints", &out_keypoints);
	
	return true;
}

bool ExtractSIFT::onFinish() {
	return true;
}

bool ExtractSIFT::onStep() {
	return true;
}

bool ExtractSIFT::onStop() {
	return true;
}

bool ExtractSIFT::onStart() {
	SIFT_Detector = new cv::SiftFeatureDetector(SIFTFeatureThreshold,SIFTLinesThreshold);
	return true;
}

void ExtractSIFT::onNewImage() {
	try {
		Mat img = in_img.read();
		Mat out_img = img.clone();
		
		SiftDescriptorExtractor SIFT_Descriptor_Extractor;
		Mat descriptors;
		std::vector<cv::KeyPoint> keypoints;
		
		SIFT_Detector->detect(img, keypoints);
		SIFT_Descriptor_Extractor.compute(img, keypoints, descriptors);
		
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

}//: namespace ExtractSIFT
}//: namespace Processors
