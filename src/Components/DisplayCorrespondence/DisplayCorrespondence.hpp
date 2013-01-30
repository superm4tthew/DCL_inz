/*!
 * \file
 * \brief
 */

#ifndef DISPLAYCORRESPONDENCE_HPP_
#define DISPLAYCORRESPONDENCE_HPP_

#include "Component_Aux.hpp"
#include "Component.hpp"
#include "Panel_Empty.hpp"
#include "DataStream.hpp"


#include "opencv2/core/core.hpp"
#include "opencv2/features2d/features2d.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <cv.h>

#include "Property.hpp"

namespace Processors {
namespace DisplayCorrespondence {

using namespace cv;

/*!
 * \class DisplayCorrespondence
 * \brief DisplayCorrespondence processor class.
 */
class DisplayCorrespondence: public Base::Component
{
public:
	/*!
	 * Constructor.
	 */
	DisplayCorrespondence(const std::string & name = "");

	/*!
	 * Destructor
	 */
	virtual ~DisplayCorrespondence();


protected:

	/*!
	 * Connects source to given device.
	 */
	bool onInit();

	/*!
	 * Disconnect source from device, closes streams, etc.
	 */
	bool onFinish();

	/*!
	 * Retrieves data from device.
	 */
	bool onStep();

	/*!
	 * Start component
	 */
	bool onStart();

	/*!
	 * Stop component
	 */
	bool onStop();
	
	void onNewImage();
	
	Base::DataStreamIn<std::vector<int> > in_image_params;
	Base::DataStreamIn<Mat> in_img;
	Base::DataStreamIn<std::string> in_path;
	Base::DataStreamIn<std::vector<std::string> > in_all_file_paths;
	
	Base::DataStreamIn<std::vector<std::vector<int> > > in_MatchedSourceForTile;
	Base::DataStreamIn< std::vector<int> > in_PositionOfPatchesInImages;
	Base::DataStreamIn<std::vector<std::vector<int> > > in_MatchedPatchInMatcher;
	Base::DataStreamIn<std::vector<std::vector<double> > > in_DistanceMap;
	
	Base::DataStreamOut<Mat> out_image;
	
	Base::DataStreamIn<std::vector<double> > in_match_map;
	Base::DataStreamIn<double > in_match_quality;

	Base::EventHandler<DisplayCorrespondence> h_onNewImage;
	Base::Event * matched;
	
private:

	Base::Property<int> mode;
	Base::Property<double> scalefactor;
	Base::Property<double> angle;
	
	std::vector<int> wrongmatches;
	std::vector<int> goodmatches;
	std::vector<int> goodmatches_q;
	std::vector<double> similarity;
	
	int patches_cols;
	int patches_rows;
};

}//: namespace DisplayCorrespondence
}//: namespace Processors


/*
 * Register processor component.
 */
REGISTER_PROCESSOR_COMPONENT("DisplayCorrespondence", Processors::DisplayCorrespondence::DisplayCorrespondence, Common::Panel_Empty)

#endif /* DISPLAYCORRESPONDENCE_HPP_ */
