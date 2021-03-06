#include <memory>
#include <string>

#include "Utils.hpp"
//#include <algorithm> //chyba nei potrzerbne
//#include <boost/foreach.hpp> //jw
#include "MatcherFLANN.hpp"
#include "Common/Logger.hpp"

namespace Processors {
namespace MatcherFLANN {

MatcherFLANN::MatcherFLANN(const std::string & name) :
	Base::Component(name), directory("directory", std::string(".")), pattern(
			"pattern", std::string(".*\\.yml")), savematcher("savematcher",
			std::string(".")), patchsize("patchsize", 32), mode("mode", 0),
			threshold("threshold", 0.2), queuesize("queuesize", 10), matching_mode("matching_mode", 0), matching_param("matching_param", 1), advanced_options("advanced_options",1), min_tile_matches("min_tile_matches",1) {

	registerProperty( directory);
	registerProperty( pattern);
	registerProperty( savematcher);
	registerProperty( patchsize);
	registerProperty( mode);
	registerProperty( threshold);
	registerProperty( queuesize);
	registerProperty( matching_mode);
	registerProperty( matching_param);
	registerProperty( min_tile_matches);
}

MatcherFLANN::~MatcherFLANN() {
}

bool MatcherFLANN::onInit() {
	LOG(LTRACE) << "MatcherFLANN::initialize\n";
	matched = registerEvent("matched");
	h_onNewImage.setup(this, &MatcherFLANN::onNewImage);
	registerHandler("onNewImage", &h_onNewImage);

	registerStream("matched_img", &matched_img);
	registerStream("in_descriptors", &in_descriptors);
	registerStream("in_keypoints", &in_keypoints);
	registerStream("in_to_match", &in_to_match);
	registerStream("out_path", &out_path);
	registerStream("out_image_params", &out_image_params);
	
	registerStream("out_MatchedSourceForTile", &out_MatchedSourceForTile);
	registerStream("out_PositionOfPatchesInImages", &out_PositionOfPatchesInImages);
	registerStream("out_MatchedPatchInMatcher", &out_MatchedPatchInMatcher);
	registerStream("out_DistanceMap", &out_DistanceMap);
	registerStream("out_all_file_paths", &out_all_file_paths);
	
	registerStream("out_match_map", &out_match_map);
	registerStream("out_match_quality", &out_match_quality);
	
	return true;
}

bool MatcherFLANN::onFinish() {
	return true;
}

bool MatcherFLANN::onStep() {
	return true;
}

bool MatcherFLANN::onStop() {
	return true;
}

bool MatcherFLANN::onStart() {
	ctr = 0;
	num_loops = 0;
	queue_size = queuesize;
	findFiles();
	readDB();
	files_number = files.size();

	flannmatcher.train();
	return true;
}

void MatcherFLANN::onNewImage() {
	Mat descriptors = in_descriptors.read();
	Mat image = in_to_match.read();
	std::vector<cv::KeyPoint> keypoints = in_keypoints.read();

	//this values will be increased when images gets a vote
	int VoteMap[files_number];
	for (int i = 0; i < files_number; i++) {
		VoteMap[i] = 0;
	}

	std::vector<cv::DMatch> BestMatches;
	int error = 0, sanity_check = 0;
	Mat imageMatches;

	//query image size in px
	int image_cols = image.cols;
	int image_rows = image.rows;

	//quary image size in patches
	int patches_cols = image_cols / patchsize;
	int patches_rows = image_rows / patchsize;

	//image map dimensions
	int map_size_x = patches_cols * patchsize;
	int map_size_y = patches_rows * patchsize;

	//from which DB image this patch comes from
	//int MatchedSourceForTile[patches_rows * patches_cols][queue_size];
	std::vector<std::vector<int> > MatchedSourceForTile;
	
	//patch position in classifier
	//int MatchedPatchInMatcher[patches_rows * patches_cols][queue_size];
	std::vector<std::vector<int> > MatchedPatchInMatcher;
	
	//map of distances to the best matched keypoint
	std::vector<std::vector<double> > DistanceMap;
	//int DistanceMap[patches_rows * patches_cols][queue_size];
	std::vector<double> QueryMatchMap;
	

	MatchedSourceForTile.resize(patches_cols*patches_rows);
	MatchedPatchInMatcher.resize(patches_cols*patches_rows);
	DistanceMap.resize(patches_cols*patches_rows);
	QueryMatchMap.resize(patches_cols*patches_rows);
	
	
	//prepare map for matches
	for (int k = 0; k < patches_rows * patches_cols; k++) {
		for (int zzz = 0; zzz < queue_size; zzz++) {
			MatchedSourceForTile[k].push_back(-1);
			MatchedPatchInMatcher[k].push_back(-1);
			DistanceMap[k].push_back(-1);
		}
	}
	
	//keypoitns for each patch
	std::vector<std::vector<cv::KeyPoint> > KeypointMap;

	

	//prepare needed space
	KeypointMap.resize(patches_cols * patches_rows);
	//DistanceMap.resize(patches_cols * patches_rows);

	//descriptors for each patch
	std::vector<Mat*> DescriptorsMap;

	//reserve sufficient space for descriptors
	for (int i = 0; i < patches_cols * patches_rows; i++) {
		DescriptorsMap.push_back(new Mat(0, 64, CV_32F));
	}

	int tilemap_x, tilemap_y;

	for (int i = 0; i < keypoints.size(); i++) {
		//check if keypoints of incomming image fall in the grid
		if (keypoints[i].pt.x >= map_size_x || keypoints[i].pt.y >= map_size_y) {
			continue;
		}
		//to which patch KP falls
		tilemap_x = keypoints[i].pt.x / patchsize;
		tilemap_y = keypoints[i].pt.y / patchsize;

		//add it to the map
		KeypointMap[patches_cols * tilemap_y + tilemap_x].push_back(
				keypoints[i]);
		DescriptorsMap[patches_cols * tilemap_y + tilemap_x]->push_back(
				descriptors.row(i));
	}

	//match each patch
	for (int i = 0; i < patches_cols * patches_rows; i++) {
		//set the distance to big value
		for (int zzz = 0; zzz < queue_size; zzz++) {
			DistanceMap[i][zzz] = 1000;
		}
		
		//if no keypoints in this patch then there is nothing to match :)
		if (KeypointMap[i].size() <= 0) {
			QueryMatchMap[i]=0.0;
			continue;
		}

		//do matching
		flannmatcher.match(*DescriptorsMap[i], BestMatches);

		//sort matches
		sort(BestMatches.begin(), BestMatches.end());

		//if matches are not good enough then get rid of them
		//if (BestMatches[0].distance > threshold) {
		//	continue;
		//}

		

		//fill map of matches. For each patch keep a limited queue of best matches
		for (int zzz = 0; zzz < queue_size && zzz < KeypointMap[i].size(); zzz++) {
			if (BestMatches[zzz].distance > threshold) {
				break;
			}
		
			MatchedSourceForTile[i][zzz]
					= GlobalMappingPatchToImage[BestMatches[zzz].imgIdx];
			MatchedPatchInMatcher[i][zzz] = BestMatches[zzz].imgIdx;
			
			//update distance to the best match
			DistanceMap[i][zzz] = BestMatches[zzz].distance;
		}
		
		int best_fit=-3;
		
		if (matching_mode==0){			//single best fit
			best_fit=BestMatches[0].imgIdx;
			//QueryMatchMap[i]=(threshold-BestMatches[0].distance)/threshold;
			QueryMatchMap[i]=1;
			std::cout<<"m:"<<(threshold-BestMatches[0].distance)/threshold<<std::endl;
		}
		else if (matching_mode==1){		//appearing most often out of matching_param
			int max_size=KeypointMap[i].size()>matching_param?matching_param:KeypointMap[i].size();
			int best_matches[max_size];
			int max_mini_votes=0;
			std::vector<int> possible_values;
			for (int k=0; k<max_size; k++){
				best_matches[k]=0;
			}
			for (int k=0; k<max_size; k++){
				//if (possible_values.size()==0){
				//	possible_values.push_back(BestMatches[k].imgIdx);
				//	best_matches[k]++;
				//	continue;
				//}
				int cntr=0;
				int j=0;
				for (j=0; j<possible_values.size(); j++){
					if (possible_values[j]==BestMatches[k].imgIdx){
						best_matches[j]++;
						cntr++;
						break;
					}
				}
				if (cntr==0){
					possible_values.push_back(BestMatches[k].imgIdx);
					best_matches[j]++;
				}
	
			}
			//continuous
			
			//	std::cout<<"posval:"<<possible_values.size()<<std::endl;
				//for (int qw=0; qw<possible_values.size(); qw++){
			//		if (GlobalMappingPatchToImage[possible_values[qw]]!=8){
			//			std::cout<<"   pv:"<<possible_values[qw]<<" bm:"<<best_matches[qw]<<" img:"<<GlobalMappingPatchToImage[possible_values[qw]]<<std::endl;
			//		}
			//	}
			
			for (int k=0; k<max_size; k++){
				if (max_mini_votes<best_matches[k] && best_matches[k]>=min_tile_matches){
					max_mini_votes=best_matches[k];
					best_fit=possible_values[k];
				}
			}
			
			QueryMatchMap[i]=(double)max_mini_votes/max_size;
			//cout<<max_mini_votes<<" "<<max_size<<" "<<QueryMatchMap[i]<<std::endl;
			
				//if (GlobalMappingPatchToImage[best_fit]!=8){
				//	for (int qw=0; qw<possible_values.size(); qw++){
				//		std::cout<<"    qw:"<<qw<<" pv:"<<possible_values[qw]<<" bm:"<<best_matches[qw]<<" img:"<<GlobalMappingPatchToImage[possible_values[qw]]<<std::endl;
				//	}
				//}
			
			//if (max_mini_votes<0){
			//	LOG(LTRACE) << "MatcherFLANN::mini_votes wrong value\n";
			//	return;
			//}
			//std::cout<<"bestfit:"<<best_fit<<" v:"<<max_mini_votes<<
			//best_fit=best_matches[max_mini_votes];
		}
		else if (matching_mode==2){
		
		}
		else{
			LOG(LTRACE) << "MatcherFLANN::matching_mode unsupported value\n";
			return;
		}
		//std::cout<<"bestfit"<<best_fit<<"  img:"<<GlobalMappingPatchToImage[best_fit]<<std::endl;
		
		if (best_fit>=0){
			//vote for image
			VoteMap[GlobalMappingPatchToImage[best_fit]]++;

			//count number of votes
			sanity_check++;
		}

		//clear best matches
		BestMatches.clear();
	}

	//check which IMG got the most votes
	int max = -1;
	int BestMatchingImage;
	for (int k = 0; k < files.size(); k++) {
		//std::cout<<VoteMap[k]<<std::endl;
		if (VoteMap[k] > max) {
			max = VoteMap[k];
			BestMatchingImage = k;
		}
	}
	
	double matchquality=0;
	for (int k=0; k<QueryMatchMap.size(); k++){
		matchquality+=QueryMatchMap[k]/(patches_cols * patches_rows);
	}
	
	std::cout<<"matchq"<<matchquality<<" advo:"<<advanced_options<<" thr"<<threshold<<" ti"<<(int)1000*threshold<<std::endl;
	
	out_match_quality.write(matchquality);
	/**********************************************************************/
	
	
	
	
		int bd_cols = BDImageSizeCols[BestMatchingImage];
		int bd_rows = BDImageSizeRows[BestMatchingImage];
		int bd_patch_size = BDImagePatchSize[BestMatchingImage];
		
		
		
	
		std::vector<int> image_params;
		image_params.push_back(patches_cols);
		image_params.push_back(patches_rows);
		image_params.push_back(patchsize);
	
		image_params.push_back(bd_cols);
		image_params.push_back(bd_rows);
		image_params.push_back(bd_patch_size);
	
		image_params.push_back(queue_size);
		image_params.push_back(BestMatchingImage);
	
		image_params.push_back(image_cols);
		image_params.push_back(image_rows);
		
		image_params.push_back(threshold*10000);
	
		//image_params.push_back(image.cols);
		//image_params.push_back(image.rows);
		
	
	
	
		out_image_params.write(image_params);
		out_path.write(all_file_paths[BestMatchingImage]);
	
	
	
		out_MatchedSourceForTile.write(MatchedSourceForTile);
		out_PositionOfPatchesInImages.write(PositionOfPatchesInImages);
		out_MatchedPatchInMatcher.write(MatchedPatchInMatcher);
		out_DistanceMap.write(DistanceMap);
	
		out_match_map.write(QueryMatchMap);
	
		out_all_file_paths.write(all_file_paths);
	
	
	//where in the image is the patch (all patches are numbered linearly in 1D. Like in progressive scan in TV from left to right and to next line)
	//PositionOfPatchesInImages
	
	//patch position in classifier
	//MatchedPatchInMatcher
	
	//from which DB image this patch comes from
	//MatchedSourceForTile
	
	//keypoitns for each patch
	//KeypointMap
	
	//descriptors for each patch
	//DescriptorsMap
	
	//to which file in DB belongs each tile
	//GlobalMappingPatchToImage

	
	matched->raise();
}

void MatcherFLANN::readDB() {
	int file_ctr = 0;
	for (int i = 0; i < files.size(); i++) {
		FileStorage fs((files[i]).c_str(), FileStorage::READ);

		int patches_cols;
		fs["patches_cols"] >> patches_cols;
		int patches_rows;
		fs["patches_rows"] >> patches_rows;
		int tile_size;
		fs["tile_size"] >> tile_size;

		BDImageSizeCols.push_back(patches_cols);
		BDImageSizeRows.push_back(patches_rows);
		BDImagePatchSize.push_back(tile_size);

		//READ NUM
		//int temp;
		//fs["ID"] >> temp;

		//READ PATH
		std::string fpath;
		fs["file_name"] >> fpath;

		for (int r = 0; r < patches_rows * patches_cols; r++) {
			ostringstream oss;
			oss << "kp" << r;
			std::vector<cv::KeyPoint> keypoints;
			read(fs[oss.str()], keypoints);
			if (keypoints.size() == 0) {
				continue;
			}
			//to which file in DB belongs each tile
			GlobalMappingPatchToImage.push_back(i);
			PositionOfPatchesInImages.push_back(r);
			all_keypoints.push_back(keypoints);
		}
		for (int r = 0; r < patches_rows * patches_cols; r++) {
			ostringstream oss;
			oss << "dsc" << r;
			Mat descriptors;
			read(fs[oss.str()], descriptors);
			if (descriptors.rows == 0) {
				continue;
			}
			all_descriptors.push_back(descriptors);
		}

		//RELEASE
		fs.release();
		all_file_paths.push_back(fpath);
	}
	flannmatcher.add(all_descriptors);
}

/*
 * Find files in database directory
 */
bool MatcherFLANN::findFiles() {
	files.clear();
	files = Utils::searchFiles(directory, pattern);
	BOOST_FOREACH(std::string fname, files) {
	}
	return !files.empty();
}

}//: namespace MatcherFLANN
}//: namespace Processors
