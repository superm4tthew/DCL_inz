/*!
 * \file
 * \brief
 */

#ifndef Sequencer_HPP_
#define Sequencer_HPP_

#include "Component_Aux.hpp"
#include "Component.hpp"
#include "Panel_Empty.hpp"
#include "DataStream.hpp"
#include "Property.hpp"

#include <vector>
#include <string>

#include <cv.h>

namespace Processors {
namespace Sequencer {

using namespace cv;

/*!
 * \class Sequencer
 * \brief Sequencer processor class.
 */
class Sequencer: public Base::Component {
public:
	/*!
	 * Constructor.
	 */
	Sequencer(const std::string & name = "");

	/*!
	 * Destructor
	 */
	virtual ~Sequencer();

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

	/// Event signaling that new image was retrieved.
	Base::Event * newImage;

	/// Sequence has ended
	Base::Event * endOfSequence;

	/// Output data stream
	Base::DataStreamOut<Mat> out_img;
	Base::DataStreamOut<std::string> file_path;
	Base::DataStreamOut< std::vector<int> > img_size;

	/*!
	 * Event handler function.
	 */
	void onTrigger();

	/// Event handler.
	Base::EventHandler<Sequencer> h_onTrigger;

private:
	/**
	 * Fill list of files according to pattern
	 *
	 * \return true, if there is at least one file found, false otherwise
	 */
	bool findFiles();

	/// list of files in sequence
	std::vector<std::string> files;

	std::vector<cv::Mat> images;

	/// current frame
	cv::Mat img;

	/// index of current frame
	int frame;

	bool trig;
	std::vector<int> image_size;

	Base::Property<std::string> directory;
	Base::Property<std::string> pattern;
	Base::Property<int> sort;
	Base::Property<int> prefetch;
	Base::Property<int> triggered;
	Base::Property<int> loop;
	Base::Property<int> files_to_load;
	

};

}//: namespace Sequencer
}//: namespace Processors


/*
 * Register processor component.
 */
REGISTER_PROCESSOR_COMPONENT("Sequencer", Processors::Sequencer::Sequencer, Common::Panel_Empty)

#endif /* Sequencer_HPP_ */
