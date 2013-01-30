/*!
 * \file
 * \brief
 */

#include <memory>
#include <string>
#include <time.h>
#include <iostream>
#include <sstream>

#include <fstream>

#include <fstream>
#include "DisplayCorrespondence.hpp"
#include "Common/Logger.hpp"

namespace Processors {
namespace DisplayCorrespondence {

DisplayCorrespondence::DisplayCorrespondence(const std::string & name) :
	Base::Component(name), mode("mode", 0), scalefactor("scalefactor", 1.0), angle("angle", 0.0) {
	LOG(LTRACE) << "Hello DisplayCorrespondence\n";

	registerProperty( mode);
	registerProperty( scalefactor);
	registerProperty( angle);
}

DisplayCorrespondence::~DisplayCorrespondence() {
	LOG(LTRACE) << "Good bye DisplayCorrespondence\n";
}

bool DisplayCorrespondence::onInit() {

	matched = registerEvent("matched");
	h_onNewImage.setup(this, &DisplayCorrespondence::onNewImage);
	registerHandler("onNewImage", &h_onNewImage);

	registerStream("in_image_params", &in_image_params);
	registerStream("in_img", &in_img);
	registerStream("in_path", &in_path);
	registerStream("out_image", &out_image);

	registerStream("in_MatchedSourceForTile", &in_MatchedSourceForTile);
	registerStream("in_PositionOfPatchesInImages",
			&in_PositionOfPatchesInImages);
	registerStream("in_MatchedPatchInMatcher", &in_MatchedPatchInMatcher);
	registerStream("in_DistanceMap", &in_DistanceMap);
	registerStream("in_all_file_paths", &in_all_file_paths);

	registerStream("in_match_map", &in_match_map);
	registerStream("in_match_quality", &in_match_quality);

	LOG(LTRACE) << "DisplayCorrespondence::initialize\n";

	// Register data streams, events and event handlers HERE!

	return true;
}

bool DisplayCorrespondence::onFinish() {
	LOG(LTRACE) << "DisplayCorrespondence::finish\n";

	return true;
}

bool DisplayCorrespondence::onStep() {
	LOG(LTRACE) << "DisplayCorrespondence::step\n";
	return true;
}

bool DisplayCorrespondence::onStop() {

	ofstream myfile;
  	myfile.open ("/home/mati/Desktop/rotmatch.txt");
  	if (mode==4){
	  	for (int i=0; i< wrongmatches.size(); i++){
			myfile<<i<<"\t"<<wrongmatches[i]<<"\t"<<goodmatches[i]<<"\t"<<similarity[i]<<"\t"<<(double)goodmatches[i]/(patches_cols*patches_rows)<<"\t"<<(double)wrongmatches[i]/(patches_cols*patches_rows)<<std::endl; //<<goodmatches_q[i]
	  	}
	  	myfile.close();
	}
	return true;
}

bool DisplayCorrespondence::onStart() {
	srand(time(NULL));
	return true;
}

void DisplayCorrespondence::onNewImage() {
	int wrongmatch_counter=0;
	int goodmatch_counter=0;

	Mat image = in_img.read();
	std::string path = in_path.read();
	std::vector<int> image_params = in_image_params.read();
	std::vector<std::vector<int> > MatchedSourceForTile =in_MatchedSourceForTile.read();
	std::vector<int> PositionOfPatchesInImages =in_PositionOfPatchesInImages.read();
	std::vector<std::vector<int> > MatchedPatchInMatcher =in_MatchedPatchInMatcher.read();
	std::vector<std::vector<double> > DistanceMap = in_DistanceMap.read();
	std::vector<std::string> all_file_paths = in_all_file_paths.read();
	int files_number = all_file_paths.size();
	std::vector<double> QueryMatchMap = in_match_map.read();
	double match_quality = in_match_quality.read();

	patches_cols = image_params[0];
	patches_rows = image_params[1];
	int patchsize = image_params[2];

	int bd_cols = image_params[3];
	int bd_rows = image_params[4];
	int bd_patch_size = image_params[5];

	int queue_size = image_params[6];
	int BestMatchingImage = image_params[7];

	int image_cols = image_params[8];
	int image_rows = image_params[9];
	
	double threshold = (double)image_params[10]/10000;

	std::vector<std::vector<int> > BDMatchMap;
	BDMatchMap.resize(bd_cols * bd_rows);
	//for (int q = 0; q < bd_cols * bd_rows; q++) {
	//	BDMatchMap[q] = -1;
	//}

	int query_col;
	int query_row;
	int fn;

	std::cout << "qs:" << queue_size << "  thre:"<<threshold<<" p:"<<image_params[10]<<std::endl;
	/*=======================================================================*/
	/*************************************************************************/
	/*		create corresspondence map for DB image							 */
	/*************************************************************************/

	if (mode == 3) {
		for (int k = 0; k < patches_rows * patches_cols; k++) {
			int flag = 0;
			int zzz;
			//check if there is match of current patch to the best image
			for (zzz = 0; zzz < queue_size; zzz++) {
				if (DistanceMap[k][zzz]>threshold || DistanceMap[k][zzz]<0){
					//std::cout<<"k:"<<k<<" zzz:"<<zzz<<" dmap"<<DistanceMap[k][zzz]<<"  th:"<<threshold<<std::endl;
					//break;
				}
				if (MatchedSourceForTile[k][zzz] == BestMatchingImage) {
					flag = 1;
					break;
				}
			}
			if (flag) {
				//where in the image is the patch (all patches are numbered linearly in 1D. Like in progressive scan in TV from left to right and to next line)
				fn = PositionOfPatchesInImages[MatchedPatchInMatcher[k][zzz]];
				BDMatchMap[fn].push_back(k);
			}
		}
	}
	
	if (mode==4){
	//!!!!!!!!!!!!!!!!!!!!!!!
	Mat matchedim = cv::imread(path, -1);
	//!!!!!!!!!!!!!!!!!!
		double nx, ny, mx, my;
		
		double angle_rad=M_PI*angle/180;
		
		//scalefactor=1.0;
		int basex=bd_patch_size*bd_cols;
		int basey=bd_patch_size*bd_rows;
		//int imgx=patchsize*patches_cols;
		//int imgy=patchsize*patches_rows;
		int imgx=image_cols;
		int imgy=image_rows;
				
		//circle(matchedim, Point(basex/2.0,basey/2.0), 20, Scalar(0,255,0),-1);
		//circle(image, Point(imgx/2.0,imgy/2.0), 20, Scalar(0,255,0),-1);
		//float px=-100, py=-100;
		
		//float px2=px*scalefactor;
		//float py2=px*scalefactor;
		
		//float qx=px2*cos(angle_rad)-py2*sin(angle_rad);
		//float qy=px2*sin(angle_rad)+py2*cos(angle_rad);
		
		
		
		//std::cout<<qx<<" "<<qy<<std::endl;
		
		//circle(matchedim, Point(basex/2.0+px,basey/2.0+py), 10, Scalar(255,0,0),-1);
		//circle(image, Point(basex/2.0+qx,basey/2.0+qy), 10, Scalar(255,0,0),-1);
				
		for (int k = 0; k < patches_rows * patches_cols; k++) {
					
					
			int flag = 0;
			int zzz;
			//check if there is match of current patch to the best image
			for (zzz = 0; zzz < queue_size; zzz++) {
				if (DistanceMap[k][zzz]>threshold || DistanceMap[k][zzz]<0.0){
					//std::cout<<"k:"<<k<<" zzz:"<<zzz<<" dmap"<<DistanceMap[k][zzz]<<"  th:"<<threshold<<std::endl;
					break;
				}
				if (MatchedSourceForTile[k][zzz] == BestMatchingImage) {
					flag = 1;
					break;
				}
			}
			if (flag) {
			
				fn = PositionOfPatchesInImages[MatchedPatchInMatcher[k][zzz]];
			
				
		
				//float p1x=base_keypoints[matches[i].queryIdx].pt.x-basex/2;
				//float p1y=base_keypoints[matches[i].queryIdx].pt.y-basey/2;
				//float p2x=keypoints[matches[i].trainIdx].pt.x-imgx/2;
				//float p2y=keypoints[matches[i].trainIdx].pt.y-imgy/2;
				
				//cout<<"loop i2: "<<i<<std::endl;
				//float bigx=p1x*(cos(angle_rad)-sin(angle_rad));
				//float bigy=p1y*(sin(angle_rad)+cos(angle_rad));
				
				//float bigx=p1x*cos(angle_rad)-p1y*sin(angle_rad);
				//float bigy=p1x*sin(angle_rad)+p1y*cos(angle_rad);
				
				//cout<<"loop i3: "<<i<<std::endl;
				
				//if (sqrt((bigx-p2x)*(bigx-p2x)+(bigy-p2y)*(bigy-p2y))<=threshold){
					
			
				nx = bd_patch_size * (fn % bd_cols) + bd_patch_size * 0.5;
				ny = bd_patch_size * (fn / bd_cols) + bd_patch_size * 0.5;
				
				//nx*=scalefactor;
				//ny*=scalefactor;
				
				//which tile is matched
				query_col = k % patches_cols;
				query_row = k / patches_cols;

				//location of the center
				mx = patchsize * (query_col) + 0.5 * patchsize;
				my = patchsize * (query_row) + 0.5 * patchsize;
				
				
				float p1x=nx-basex/2.0;
				float p1y=ny-basey/2.0;
				float p2x=mx-imgx/2.0;
				float p2y=my-imgy/2.0;
				
				p1x*=scalefactor;
				p1y*=scalefactor;
				
				//circle(image, Point(mx,my), 10, Scalar(0,0,255), -1);
				//circle(matchedim, Point(nx,ny), 10, Scalar(0,0,255),-1);
				
				
				
				
				float bigx=p1x*cos(angle_rad)-p1y*sin(angle_rad);
				float bigy=p1x*sin(angle_rad)+p1y*cos(angle_rad);	//<<" basex"<<basex<<" basey"<<basey<<" imgx"<<imgx<<" imgy"<<imgy
				//std::cout<<"scale:"<<scalefactor<<std::endl;
				
				if (sqrt((bigx-p2x)*(bigx-p2x)+(bigy-p2y)*(bigy-p2y))<=(sqrt(2)*bd_patch_size*scalefactor/2+3)){
					//where in the image is the patch (all patches are numbered linearly in 1D. Like in progressive scan in TV from left to right and to next line)
					
					//std::cout<<"p1x:"<<p1x<<" p1y:"<<p1y<<" p2x:"<<p2x<<" p2y:"<<p2y<<" bx:"<<bigx<<" by:"<<bigy<<" scale:"<<scalefactor<<std::endl;
				//std::cout<<sqrt((bigx-p2x)*(bigx-p2x)+(bigy-p2y)*(bigy-p2y))<<std::endl;
					
					BDMatchMap[fn].push_back(k);
					goodmatch_counter++;
				}
				else{
					wrongmatch_counter++;
				}
			}
	
		}
		wrongmatches.push_back(wrongmatch_counter);
		goodmatches.push_back(goodmatch_counter);
		similarity.push_back(match_quality);
	}
	/*=======================================================================*/
	/*************************************************************************/
	/*				simple correspondence drawing							 */
	/*************************************************************************/
	
	if (mode == 0) {
		float nx, ny, mx, my;
		Mat outimg;
		//int query_col;
		//int query_row;


		std::vector<cv::KeyPoint> im1kp;
		std::vector<cv::KeyPoint> im2kp;
		std::vector<cv::DMatch> immatches;

		int tempc = 0;

		int cant = 0, cant2 = 0;
		//read query image
		Mat matchedim = cv::imread(path, -1);
		double color;

		for (int k = 0; k < patches_rows * patches_cols; k++) {
			int flag = 0;
			int zzz;
			//check if there is match of current patch to the best image
			for (zzz = 0; zzz < queue_size; zzz++) {
				if (MatchedSourceForTile[k][zzz] == BestMatchingImage) {
					flag = 1;
					//cant++;
					break;
				}
			}
			if (flag) {

				//where in the image is the patch (all patches are numbered linearly in 1D. Like in progressive scan in TV from left to right and to next line)
				fn = PositionOfPatchesInImages[MatchedPatchInMatcher[k][zzz]];
				//BDMatchMap[fn] = k;

				//error?
				if (fn < 0) {
					return;
				}

				//location of the center tile in the image#include <time.h>
				nx = bd_patch_size * (fn % bd_cols) + bd_patch_size * 0.5;
				ny = bd_patch_size * (fn / bd_cols) + bd_patch_size * 0.5;

				//which tile is matched
				query_col = k % patches_cols;
				query_row = k / patches_cols;

				//location of the center
				mx = patchsize * (query_col) + 0.5 * patchsize;
				my = patchsize * (query_row) + 0.5 * patchsize;

				//Is it not a bad match?
				if (DistanceMap[k][0] < 0 || DistanceMap[k][0] > 0.3) {
					cant2++;
					continue;
				}

				//choose colour
				//color = (1 - 10 * DistanceMap[k][0] * DistanceMap[k][0]) * 255.0;
				int cb = 0, cg = 0, cr = 255;
				int qLineW = 2, bdLineW = 2;
				//cb=rand() % 255;
				//cg=rand() % 255;
				//cr=rand() % 255;

				//draw patches in query img
				rectangle(image, Point(mx - 0.5 * patchsize, my - 0.5
						* patchsize), Point(mx + 0.5 * patchsize, my + 0.5
						* patchsize), Scalar(cb, cg, cr), qLineW);

				//draw patches in BD img
				rectangle(matchedim, Point(nx - 0.5 * bd_patch_size, ny - 0.5
						* bd_patch_size), Point(nx + 0.5 * bd_patch_size, ny
						+ 0.5 * bd_patch_size), Scalar(cb, cg, cr), bdLineW);

				//each tile center as a KP
				im1kp.push_back(KeyPoint(Point2f(nx, ny), 5.0));
				im2kp.push_back(KeyPoint(Point2f(mx, my), 5.0));

				std::ostringstream q;
				q.precision(2);
				q << QueryMatchMap[k];

				//putText(image, q.str(), cvPoint(mx-0.25*patchsize,my-0.125*patchsize), FONT_HERSHEY_SIMPLEX, 0.5, cvScalar(0,0,0), 1.5, CV_AA);

				//and now set correspondence
				immatches.push_back(DMatch(tempc, tempc, 0.0));
				tempc++;
			}
		}
		drawMatches(image, im2kp, matchedim, im1kp, immatches, outimg,
				Scalar::all(-1));
		out_image.write(outimg);
	}

	/*=======================================================================*/
	/*************************************************************************/
	/*				corresponding tiles marked with the same color			 */
	/*************************************************************************/
	if (mode == 3 || mode==4) {
		std::cout << "mode:" << mode << " s" << BDMatchMap.size() << std::endl;
		//!!!!!!!!!!!!!!!!!!!
		//!!!!!!!!!!!!!!!!
		//int counter=0;
		Mat matchedim = cv::imread(path, -1);
		float nx, ny, mx, my;
		Mat outimg;
		std::vector<cv::KeyPoint> im1kp;
		std::vector<cv::KeyPoint> im2kp;
		std::vector<cv::DMatch> immatches;

		int k;
		for (int m = 0; m < BDMatchMap.size(); m++) {
			int cb = 0, cg = 0, cr = 255;
			int qLineW = -1, bdLineW = -1;
			cb = rand() % 255;
			cg = rand() % 255;
			cr = rand() % 255;
			if (BDMatchMap[m].size() > 0) {
				for (int n = 0; n < BDMatchMap[m].size(); n++) {
				
				
					k = BDMatchMap[m][n];
					fn = n;

					query_col = k % patches_cols;
					query_row = k / patches_cols;

					//std::cout<<query_col<<" "<<query_row<<" "<<k<<std::endl;

					//location of the center
					mx = patchsize * (query_col) + 0.5 * patchsize;
					my = patchsize * (query_row) + 0.5 * patchsize;

					//Is it not a bad match?
					if (DistanceMap[k][0] < 0.0 || DistanceMap[k][0] > threshold) {
						continue;
					}
					//counter++;
					//draw patches in query img
					rectangle(image, Point(mx - 0.5 * patchsize, my - 0.5
							* patchsize), Point(mx + 0.5 * patchsize, my + 0.5
							* patchsize), Scalar(cb, cg, cr), qLineW);
							
					std::ostringstream q;
					q.precision(2);
					q << QueryMatchMap[k];
					putText(image, q.str(), cvPoint(mx-0.25*patchsize,my-0.125*patchsize), FONT_HERSHEY_SIMPLEX, 0.3, cvScalar(0,0,0), 1.5, CV_AA);
							
				}

				nx = bd_patch_size * (m % bd_cols) + bd_patch_size * 0.5;
				ny = bd_patch_size * (m / bd_cols) + bd_patch_size * 0.5;

				//draw patches in BD img
				rectangle(matchedim, Point(nx - 0.5 * bd_patch_size, ny - 0.5
						* bd_patch_size), Point(nx + 0.5 * bd_patch_size, ny
						+ 0.5 * bd_patch_size), Scalar(cb, cg, cr), bdLineW);
			}
		}
		drawMatches(image, im2kp, matchedim, im1kp, immatches, outimg,
				Scalar::all(-1));

		out_image.write(outimg);
		//goodmatches_q.push_back(counter);
	}

	/*=======================================================================*/
	/*************************************************************************/
	/*			experimental mode to reconstruct image from matched patches	 */
	/*						  possible only when patches have the same size  */
	/*************************************************************************/
	//
	if ((mode == 1 && patchsize == bd_patch_size) || mode == 2) {

		int nx, ny, mx, my;
		//int fn;
		//int query_col;
		//int query_row;

		int ax, bx, ay, by;
		int fx;
		int r, g, b;
		Mat outimage(image_rows, image_cols, CV_8UC3,
				Scalar(0.0, 0.0, 0.0, 1.0));

		int counter = 0;
		int zzz = 0, flag = 0;

		//scan through all the files
		for (int l = 0; l < files_number; l++) {

			Mat matchedim = cv::imread(all_file_paths[l], -1);
			unsigned char *input = (unsigned char*) (matchedim.data);

			for (int k = 0; k < patches_rows * patches_cols; k++) {
				//matches only to the best image
				if (mode == 1) {
					flag = 0;
					//find if an image has some corresponging match on the list
					for (zzz = 0; zzz < queue_size; zzz++) {
						if (MatchedSourceForTile[k][zzz] == BestMatchingImage) {
							flag = 1;
							break;
						}
					}
				}
				//matches to all img in the DB
				if (flag || mode == 2) {

					query_col = k % patches_cols;
					query_row = k / patches_cols;
					mx = patchsize * (query_col);
					my = patchsize * (query_row);

					fn
							= PositionOfPatchesInImages[MatchedPatchInMatcher[k][zzz]];

					nx = bd_patch_size * (fn % bd_cols);
					ny = bd_patch_size * (fn / bd_cols);

					//if the tile is matched to the current image l
					if (MatchedSourceForTile[k][zzz] == l) {

						//then copy image pixel-by-pixel
						for (int ax = 0; ax < patchsize; ax++) {
							for (int ay = 0; ay < patchsize; ay++) {
								//outimage[mx+ax][mx+ay]=matchedim[nx+ax][nx+ay];
								//if ((ax+nx)>512||(ay+ny))
								b = matchedim.data[matchedim.step[0]
										* (ay + ny) + matchedim.step[1] * (ax
										+ nx)];
								g = matchedim.data[matchedim.step[0]
										* (ay + ny) + matchedim.step[1] * (ax
										+ nx) + 1];
								r = matchedim.data[matchedim.step[0]
										* (ay + ny) + matchedim.step[1] * (ax
										+ nx) + 2];

								outimage.data[outimage.step[0] * (my + ay)
										+ outimage.step[1] * (mx + ax) + 0] = b;
								outimage.data[outimage.step[0] * (my + ay)
										+ outimage.step[1] * (mx + ax) + 1] = g;
								outimage.data[outimage.step[0] * (my + ay)
										+ outimage.step[1] * (mx + ax) + 2] = r;

							}
						}
					}
				}
			}
		}

		std::cout << "done" << std::endl;
		out_image.write(outimage);
	}
	/*************************************************************************/

	std::cout << "raise" << std::endl;
	matched->raise();
}

}//: namespace DisplayCorrespondence
}//: namespace Processors
