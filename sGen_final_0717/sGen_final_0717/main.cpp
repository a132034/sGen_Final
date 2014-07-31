/********************************************************************
*																	*
*	sGen 4th project - 'see real'									*
*	안성수, 김태원, 강선우, 박도영, 이소연, 이경미, 장희수			*
*	3장의 사진을 이용하여 perspective view를 생성하는 프로그램		*
*																	*
*	last modified : 31 / 07 / 2014									*
*																	*
*	Copyright(c) 2014												*
*	All rights reserved by Do-young Park							*
*																	*
********************************************************************/
// for open cv
#include "opencv/cv.h"
#include "opencv/highgui.h"

// for image processing
#include	"dy_lib.h"
#include	"basic.h"
#include	"readPPM.h"
#include	"nonlocal.h"
#define QX_DEF_SIGMA							0.1//0.1
#define NUM_OF_INTERPOLATIED_IMAGE				8
#define MAX_DISPARITY							20

using namespace std;
void runStereo(char*filename_disparity_map, char*filename_left_image, char*filename_right_image, int max_disparity, bool use_post_processing);

INT main() // 나중에 이름 바꿔서 래핑해도 됨 
{
	//variable declaration
	register INT i, j;									//for loop
	IplImage *leftImg, *rightImg, *midImg;				//for image load
	IplImage *L, *R, *M;								//for color to gray image
	IplImage *disparity_img_L_M, *disparity_img_M_R;	//for disparity
	IplImage *resultSet[NUM_OF_INTERPOLATIED_IMAGE];	//interpolatied image 
	CvMat *disparity_L_M, *disparity_M_R;				//for normalized disparity

	//test
	IplImage *re, *re2;
	IplImage *disparity_test;
	IplImage *le, *ri;
	IplImage *view;

	char *original_left			=	"TL.jpg";
	char *original_mid = "TR.jpg";
	char *original_right = "TR.jpg";
	char *disp_l_m = "disp_l_m.ppm";
	char *disp_m_r = "disp_m_r.ppm";
	char *stereo_left = "1.ppm";
	char *stereo_mid = "6.ppm";
	char *stereo_right = "11.ppm";
	char *outcome[NUM_OF_INTERPOLATIED_IMAGE] = { "2.png", "3.png", "4.png", "5.png", "7.png", "8.png", "9.png", "10.png" };


	//test - 3장 로드 
	if ((leftImg = cvLoadImage(original_left, CV_LOAD_IMAGE_COLOR)) == NULL)
	{
		cerr << "left image is not open" << endl;
		exit(1);
	}
	if ((midImg = cvLoadImage(original_mid, CV_LOAD_IMAGE_COLOR)) == NULL)
	{
		cerr << "mid image is not open" << endl;
		exit(1);
	}
	if ((rightImg = cvLoadImage(original_right, CV_LOAD_IMAGE_COLOR)) == NULL)
	{
		cerr << "right image is not open" << endl;
		exit(1);
	}
	cout << " 1번 이미지 열기 완료!!!! (원본 이미지) " << endl;
	//원본 이미지 저장
	cvSaveImage(stereo_left, leftImg);
	cvSaveImage(stereo_mid, midImg);
	cvSaveImage(stereo_right, rightImg);
	cout << " 2번 이미지 저장 완료!!!! (원본 이미지) " << endl;
	//저장한 파일을 다시 불러와서 stereomatching (rectify 과정 우선 생략)
	runStereo(disp_l_m, stereo_left, stereo_mid, MAX_DISPARITY, false);//excluding non-local post processing
	runStereo(disp_l_m, stereo_left, stereo_mid, MAX_DISPARITY, true);//including non-local post processing

	//runStereo(disp_m_r, stereo_mid, stereo_right, MAX_DISPARITY, false);//excluding non-local post processing
	//runStereo(disp_m_r, stereo_mid, stereo_right, MAX_DISPARITY, true);//including non-local post processing
	
	//ppm 으로 열고 pgm으로 저장해서 디스패리티만 pgm으로 열면될거같음. 
	cout << " 3번 스테레오 매칭 완료!!! (디스패리티 2장 나옴!) " << endl;

	///********************************************************************************************************************************************///


	/// 3-3. disparity map과 raw data를 가지고 view interpolation(backward projection)
	// 다시 저장된 디스패리티 맵을 꺼내옴(이걸 두번 하면 됨. 
	if ((disparity_test = cvLoadImage(disp_l_m, CV_LOAD_IMAGE_UNCHANGED)) == NULL)
	{
		cerr << "disparity not open - view interpolation" << endl;
		exit(1);
	}
	cout << " 4번 디스패리티 열기 완료!!!" << endl;

	//if ((le = cvLoadImage(stereo_left, CV_LOAD_IMAGE_UNCHANGED)) == NULL)
	//{
	//	cerr << "left not open - view interpolation" << endl;
	//	exit(1);
	//}
	//if ((ri = cvLoadImage(stereo_mid, CV_LOAD_IMAGE_UNCHANGED)) == NULL)
	//{
	//	cerr << "right not open - view interpolation" << endl;
	//	exit(1);
	//}
	//
	//IplImage *rgbRe = cvCreateImage(cvSize(le->width, le->height), IPL_DEPTH_8U, 3);
	//re2 = cvCreateImage(cvSize(ri->width, ri->height), IPL_DEPTH_8U, 1);
	////char* filenames[10] = { "view1.png", "view2.png", "view3.png", "view4.png", "view5.png", "view6.png", "view7.png", "view8.png", "view9.png", "view10.png" };
	////char* filenames_c[10] = { "cview1.png", "cview2.png", "cview3.png", "cview4.png", "cview5.png", "cview6.png", "cview7.png", "cview8.png", "cview9.png", "cview10.png" };
	//
	////1. forward projection 방식 -> 완성후 backward로 바꾸고, inpaint 기술 추가 예정
	//
	//IplImage* R_Channel_le = cvCreateImage(cvGetSize(le), IPL_DEPTH_8U, 1);
	//IplImage* G_Channel_le = cvCreateImage(cvGetSize(le), IPL_DEPTH_8U, 1);
	//IplImage* B_Channel_le = cvCreateImage(cvGetSize(le), IPL_DEPTH_8U, 1);

	//IplImage* R_Channel_ri = cvCreateImage(cvGetSize(le), IPL_DEPTH_8U, 1);
	//IplImage* G_Channel_ri = cvCreateImage(cvGetSize(le), IPL_DEPTH_8U, 1);
	//IplImage* B_Channel_ri = cvCreateImage(cvGetSize(le), IPL_DEPTH_8U, 1);

	//IplImage* R_Channel_Re = cvCreateImage(cvGetSize(le), IPL_DEPTH_8U, 1);
	//IplImage* G_Channel_Re = cvCreateImage(cvGetSize(le), IPL_DEPTH_8U, 1);
	//IplImage* B_Channel_Re = cvCreateImage(cvGetSize(le), IPL_DEPTH_8U, 1);

	//cvSplit(le, B_Channel_le, G_Channel_le, R_Channel_le, 0);
	//cvSplit(ri, B_Channel_ri, G_Channel_ri, R_Channel_ri, 0);
	//cvSetZero(rgbRe);
	////cvSplit(rgbRe, B_Channel_Re, G_Channel_Re, R_Channel_Re, 0);

	//int col, row;
	//byte* disp1 = ReadPgm(&col, &row, disp_l_m);
	//cvSetZero(R_Channel_Re);
	//cvSetZero(G_Channel_Re);
	//cvSetZero(B_Channel_Re);

	//for (int n = 0; n < 9; ++n)
	//{
	//	int hole = 0;
	//	for (int c = 0; c < le->height; ++c)
	//	{
	//		for (int r = 0; r < le->width; ++r)
	//		{
	//			if ((r - disp1[c*row + r] * (9 - n) / 10) >= 0)
	//			{
	//				//re2->imageData[c* le->width + r - (disparity_test->imageData[c*le->width + r] * (n + 1) / 10)] = (le->imageData[c * le->width + r] + ri->imageData[c * le->width + r - disparity_test->imageData[c*le->width + r]]) / 2.0;
	//				R_Channel_Re->imageData[c * le->width + r - (disp1[c*row + r] * (9 - n) / 10)] =  (R_Channel_le->imageData[c * le->width + r]);// +R_Channel_ri->imageData[c * le->width + r - disp1[c*row + r]]) / 2; // B
	//				G_Channel_Re->imageData[c * le->width + r - (disp1[c*row + r] * (9 - n) / 10)] =  (G_Channel_le->imageData[c * le->width + r]);// +G_Channel_ri->imageData[c * le->width + r - disp1[c*row + r]]) / 2; // G
	//				B_Channel_Re->imageData[c * le->width + r - (disp1[c*row + r] * (9 - n) / 10)] =  (B_Channel_le->imageData[c * le->width + r]);// +B_Channel_ri->imageData[c * le->width + r - disp1[c*row + r]]) / 2; // R

	//				//R_Channel_Re->imageData[c * le->width + r - (disparity_test->imageData[c*disparity_test->widthStep + r] * (9 - n) / 10)] = (R_Channel_le->imageData[c * le->width + r]);// +R_Channel_ri->imageData[c * le->width + r - disparity_test->imageData[c * le->width + r]]) / 2; // B
	//				//G_Channel_Re->imageData[c * le->width + r - (disparity_test->imageData[c*disparity_test->widthStep + r] * (9 - n) / 10)] = (G_Channel_le->imageData[c * le->width + r]);// +G_Channel_ri->imageData[c * le->width + r - disparity_test->imageData[c * le->width + r]]) / 2; // G
	//				//B_Channel_Re->imageData[c * le->width + r - (disparity_test->imageData[c*disparity_test->widthStep + r] * (9 - n) / 10)] = (B_Channel_le->imageData[c * le->width + r]);// +B_Channel_ri->imageData[c * le->width + r - disparity_test->imageData[c * le->width + r]]) / 2; // R
	//			}
	//			else if ((r - (disp1[c*row + r] )) <= le->width)
	//			{
	//				R_Channel_Re->imageData[c * le->width + r - (disp1[c*row + r] * (9 - n) / 10)] = R_Channel_ri->imageData[c * le->width + r - (disp1[c*row + r] * (9 - n) / 10)];
	//				G_Channel_Re->imageData[c * le->width + r - (disp1[c*row + r] * (9 - n) / 10)] = G_Channel_ri->imageData[c * le->width + r - (disp1[c*row + r] * (9 - n) / 10)];
	//				B_Channel_Re->imageData[c * le->width + r - (disp1[c*row + r] * (9 - n) / 10)] = B_Channel_ri->imageData[c * le->width + r - (disp1[c*row + r] * (9 - n) / 10)];
	//				//re2->imageData[c* le->width + r - (disparity_test->imageData[c*le->width + r] * (n + 1) / 10)] = le->imageData[c * le->width + r];
	//			}
	//			else
	//			{
	//				R_Channel_Re->imageData[c * le->width + r - (disp1[c*row + r] * (9 - n) / 10)] = 0;// R_Channel_le->imageData[c * le->width + r];
	//				G_Channel_Re->imageData[c * le->width + r - (disp1[c*row + r] * (9 - n) / 10)] = 0;// G_Channel_le->imageData[c * le->width + r];
	//				B_Channel_Re->imageData[c * le->width + r - (disp1[c*row + r] * (9 - n) / 10)] = 255;// B_Channel_le->imageData[c * le->width + r];
	//				hole++;
	//			}
	//		}
	//	}
	//	cvMerge(B_Channel_Re, G_Channel_Re, R_Channel_Re, NULL, rgbRe);
	//	cout << n << " th image " << endl << "width : " << le->width << " height : " << le->height << endl << "hole / size : " << (DOUBLE)hole << " / " << le->width*le->height << endl;
	//	//cvSaveImage(filenames[n], re2);
	//	cvSaveImage(filenames_c[n], rgbRe);
	//	cvSetZero(R_Channel_Re);
	//	cvSetZero(G_Channel_Re);
	//	cvSetZero(B_Channel_Re);
	//	cvSetZero(rgbRe);
	//}
	//





	/// 3-4. inpaint하여 영상의 hole을 매꾸어 줌. 
	/// 3-5. (필터를 이용하여 블러처리 - 옵션)

	///4. 처리완료한 영상 4장씩 8장 ( left - mid 사이 4장, mid - right 사이 4장 ) 전송(저장은 옵션)








	// test - 중간 결과물 및 완성 결과물 윈도우 만들어서 보여주기
	cvNamedWindow("left original", 1); cvShowImage("left original", leftImg);
	cvNamedWindow("mid original", 1); cvShowImage("mid original", midImg);
	cvNamedWindow("right original", 1); cvShowImage("right original", rightImg);
	cvNamedWindow("1", 1); cvShowImage("1", disparity_test);
	while (1)
	{
		int c = cvWaitKey(10);
		if (c == 27)
			break;
	}

	///5. 보내준 사진 8장과 기존 3장으로 앱에서 show image ( 내가 할 부분이 아님. )





	///fin. release 
	cvReleaseImage(&leftImg);
	cvReleaseImage(&rightImg);
	cvReleaseImage(&midImg);
	cvReleaseImage(&L);
	cvReleaseImage(&M);
	cvReleaseImage(&R);
	cvReleaseImage(&disparity_img_L_M);
	cvReleaseImage(&disparity_img_M_R);

	cvReleaseMat(&disparity_L_M);
	cvReleaseMat(&disparity_M_R);
	return 0;


}


void runStereo(char*filename_disparity_map, char*filename_left_image, char*filename_right_image, int max_disparity, bool use_post_processing)
{
	double sigma = QX_DEF_SIGMA;
	unsigned char***left, ***right, **disparity;
	int h, w;

	qx_image_size(filename_left_image, h, w);//obtain image size

	left = qx_allocu_3(h, w, 3);//allocate memory
	right = qx_allocu_3(h, w, 3);
	disparity = qx_allocu(h, w);

	qx_nonlocal_cost_aggregation m_nlca;//non-local cost aggregation class
	m_nlca.init(h, w, max_disparity, sigma);//initialization

	qx_loadimage(filename_left_image, left[0][0], h, w);//load left image
	qx_loadimage(filename_right_image, right[0][0], h, w);//load right image

	qx_timer timer;//
	timer.start();

	m_nlca.matching_cost(left, right);//compute matching cost
	timer.time_display("Matching cost computation");

	timer.start();
	m_nlca.disparity(disparity, use_post_processing);//compute disparity
	if (use_post_processing) timer.time_display("Non-local cost aggregation (including non-local post processing)");
	else timer.time_display("Non-local cost aggregation (excluding non-local post processing)");

	for (int y = 0; y<h; y++) for (int x = 0; x<w; x++) disparity[y][x] *= 256 / max_disparity;//rescale the disparity map for visualization
	qx_saveimage(filename_disparity_map, disparity[0], h, w, 1);//write the obtained disparity map to the harddrive


	qx_freeu_3(left); left = NULL;//free memory
	qx_freeu_3(right); right = NULL;
	qx_freeu(disparity); disparity = NULL;
}