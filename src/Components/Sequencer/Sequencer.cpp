/*!
 * \file
 * \brief
 */

#include <memory>
#include <string>

#include "Utils.hpp"

#include <algorithm>
#include <boost/foreach.hpp>

#include <highgui.h>

#include "Sequencer.hpp"
#include "Common/Logger.hpp"

namespace Processors {
namespace Sequencer {

Sequencer::Sequencer(const std::string & name) :
	Base::Component(name), directory("directory", std::string(".")), pattern(
			"pattern", std::string(".*\\.(jpg|png|bmp)")), sort("sort", true),
			prefetch("prefetch", false), triggered("triggered", false), loop(
					"loop", false), files_to_load("files_to_load", 0) {
	LOG(LTRACE) << "Hello Sequencer\n";

	registerProperty(directory);
	registerProperty(pattern);
	registerProperty(sort);
	registerProperty(prefetch);
	registerProperty(triggered);
	registerProperty(loop);
	registerProperty(files_to_load);

	frame = 0;
	trig = false;

}

Sequencer::~Sequencer() {
	LOG(LTRACE) << "Good bye Sequencer\n";
}

bool Sequencer::onInit() {
	LOG(LTRACE) << "Sequencer::initialize\n";

	// Register data streams, events and event handlers HERE!


	h_onTrigger.setup(this, &Sequencer::onTrigger);
	registerHandler("onTrigger", &h_onTrigger);

	newImage = registerEvent("newImage");
	endOfSequence = registerEvent("endOfSequence");

	registerStream("out_img", &out_img);
	registerStream("file_path", &file_path);
	registerStream("img_size", &img_size);
	

	if (!findFiles()) {
		//LOG(LERROR) << name() << ": There are no files matching regex "	<< pattern << " in " << directory;
		return false;
	}

	if (prefetch) {
		for (int i = 0; i < files.size(); ++i) {
			images.push_back(imread(files[i], -1));
		}
	}
	image_size.resize(2);

	return true;

}

bool Sequencer::onFinish() {
	LOG(LTRACE) << "Sequencer::finish\n";

	return true;
}

bool Sequencer::onStep() {
	LOG(LTRACE) << "Sequence::onStep";
	
	//if files_to_load was set and reached then finish loading the files
	if (files_to_load != 0 && frame>=files_to_load) {
		endOfSequence->raise();
		return false;
	}

	if (triggered && !trig)
		return true;

	trig = false;

	if (frame >= files.size()) {
		LOG(LDEBUG) << "Sequence loop";
		LOG(LDEBUG) << loop;
		if (loop) {
			frame = 0;
			LOG(LDEBUG) << "Sequence loop2";
		} else {
			LOG(LINFO) << name() << ": end of sequence\n";
			endOfSequence->raise();
			return false;
		}
	}

	LOG(LTRACE) << "Sequence: reading image " << files[frame];
	try {
		//send the path of current file
		file_path.write(files[frame]);
		
		//if prefetched then just take image from the vector, i fnot load it
		if (prefetch){
			img = images[frame++];
		}
		else{
			img = cv::imread(files[frame++], -1);
		}
		image_size[0]=img.cols;
		image_size[1]=img.rows;
	} catch (...) {
		LOG(LWARNING) << name() << ": image reading failed! ["<< files[frame - 1] << "]";
	}

	out_img.write(img);
	img_size.write(image_size);
	
	newImage->raise();

	return true;
}

bool Sequencer::onStop() {
	return true;
}

bool Sequencer::onStart() {
	return true;
}

bool Sequencer::findFiles() {
	files.clear();

	files = Utils::searchFiles(directory, pattern);

	if (sort)
		std::sort(files.begin(), files.end());

	LOG(LINFO) << "Sequence loaded.";
	BOOST_FOREACH(std::string fname, files)
LOG	(LINFO) << fname;

	return !files.empty();
}

void Sequencer::onTrigger() {
	trig = true;
}

}//: namespace Sequencer
}//: namespace Processors
