/********************************************************************
*																	*
*	sGen 4th project - 'see real'									*
*	안성수, 김태원, 강선우, 박도영, 이소연, 이경미, 장희수			*
*	3장의 사진을 이용하여 perspective view를 생성하는 프로그램		*
*																	*
*	last modified : 03 / 08 / 2014									*
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
#define QX_DEF_SIGMA							0.1//0.1
#define NUM_OF_INTERPOLATIED_IMAGE				8
#define MAX_DISPARITY							20
#ifndef QX_NONLOCAL_COST_AGGREGATION_H
#define QX_NONLOCAL_COST_AGGREGATION_H
#include "treeFilter.h"

using namespace std;
void runStereo(char*filename_disparity_map, char*filename_left_image, char*filename_right_image, int max_disparity, bool use_post_processing);

class qx_nonlocal_cost_aggregation
{
public:
	qx_nonlocal_cost_aggregation();
	~qx_nonlocal_cost_aggregation();
	void clean();
	int init(int h, int w, int nr_plane,
		double sigma_range = 0.1,
		double max_color_difference = 7,
		double max_gradient_difference = 2,
		double weight_on_color = 0.11
		);
	int matching_cost(unsigned char***left, unsigned char***right);
	int disparity(unsigned char**disparity, bool use_nonlocal_post_processing = false);
private:
	void matching_cost_from_color_and_gradient(unsigned char***left, unsigned char***right);
	void compute_gradient(float**gradient, unsigned char***image);
	void compute_filter_weights(unsigned char***texture);
	void filter(float**image_filtered, float**image, bool compute_weight = true);
private:
	qx_tree_filter m_tf, m_tf_right;
	int	m_h, m_w, m_nr_plane; double m_max_color_difference, m_max_gradient_difference, m_weight_on_color, m_weight_on_color_inv;
	unsigned char**m_disparity, **m_disparity_right, **m_mask_occlusion, ***m_buf_u2;
	double m_table[256], m_sigma_range;
	unsigned char***m_left, ***m_right, ****m_buf_u3, ***m_image_shifted, ***m_image_temp;
	float***m_buf_f2, **m_cost_min, **m_cost_temp, **m_cost, **m_gradient_left, **m_gradient_right, **m_gradient_shifted;
	double****m_buf_d3, ***m_cost_vol, ***m_cost_vol_right, ***m_cost_vol_backup, ***m_cost_vol_temp;
};
#endif


INT main() 
{
	//variable declaration
	register INT i, j, c, r, n;									//for loop
	IplImage *leftImg, *rightImg, *midImg;						//for image load
	BYTE *disparity_img_L_M, *disparity_img_M_R;				//for disparity
	IplImage *result;											//interpolatied image 
	INT width, height;											//for disparity size

	//test
	IplImage *disparity_test, *disparity_test2;

	//for image names
	char *original_left							=	"view5.png";
	char *original_mid							=	"view1.png";
	char *original_right						=	"view5.png";
	char *disp_l_m								=	"disp_l_m.pgm";
	char *disp_m_r								=	"disp_m_r.pgm";
	char *stereo_left							=	"1.ppm";
	char *stereo_mid							=	"6.ppm";
	char *stereo_right							=	"11.ppm";
	char *outcome[NUM_OF_INTERPOLATIED_IMAGE]	=	 { "r2.png", "r3.png", "r4.png", "r5.png", "r7.png", "r8.png", "r9.png", "r10.png" };
	char *outLeft								=	"1.png";
	char *outMid								=	"6.png";
	char *outRight								=	"11.png";

	//3장 로드 
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

	cout << " 1번 이미지 열기 완료!!!! (원본 이미지 - IplImage) " << endl;
	//원본 이미지 저장
	cvSaveImage(stereo_left, leftImg);
	cvSaveImage(stereo_mid, midImg);
	cvSaveImage(stereo_right, rightImg);
	cvSaveImage(outLeft, leftImg);
	cvSaveImage(outMid, midImg);
	cvSaveImage(outRight, rightImg);
	cout << " 2번 이미지 저장 완료!!!! (원본 이미지 - jpg) " << endl;

	//저장한 파일을 다시 불러와서 stereomatching (rectify 과정 우선 생략)
	

	//runStereo(disp_l_m, stereo_mid, stereo_left, MAX_DISPARITY, true);//including non-local post processing
	runStereo(disp_m_r, stereo_right, stereo_mid, MAX_DISPARITY, true);//including non-local post processing
	cout << " 3번 스테레오 매칭 완료!!! (디스패리티 2장 나옴! - ppm) " << endl;

	/// 3-3. disparity map과 raw data를 가지고 view interpolation(backward projection)
	// 다시 저장된 디스패리티 맵을 꺼내옴(이걸 두번 하면 됨. 	
	disparity_img_L_M = ReadPgm(&height, &width, "flowerpots_disp1.pgm");
	disparity_img_M_R = ReadPgm(&height, &width, "flowerpots_disp1.pgm");
	cout << " 4번 디스패리티 열기 완료!!!(ppm -> byte)" << endl;

	IplImage* R_Channel_le = cvCreateImage(cvGetSize(leftImg), IPL_DEPTH_8U, 1); // for left
	IplImage* G_Channel_le = cvCreateImage(cvGetSize(leftImg), IPL_DEPTH_8U, 1);
	IplImage* B_Channel_le = cvCreateImage(cvGetSize(leftImg), IPL_DEPTH_8U, 1);

	IplImage* R_Channel_mi = cvCreateImage(cvGetSize(leftImg), IPL_DEPTH_8U, 1); // for mid
	IplImage* G_Channel_mi = cvCreateImage(cvGetSize(leftImg), IPL_DEPTH_8U, 1);
	IplImage* B_Channel_mi = cvCreateImage(cvGetSize(leftImg), IPL_DEPTH_8U, 1);

	IplImage* R_Channel_ri = cvCreateImage(cvGetSize(leftImg), IPL_DEPTH_8U, 1); // for right
	IplImage* G_Channel_ri = cvCreateImage(cvGetSize(leftImg), IPL_DEPTH_8U, 1);
	IplImage* B_Channel_ri = cvCreateImage(cvGetSize(leftImg), IPL_DEPTH_8U, 1);

	IplImage* R_Channel_Re = cvCreateImage(cvGetSize(leftImg), IPL_DEPTH_8U, 1); // for result
	IplImage* G_Channel_Re = cvCreateImage(cvGetSize(leftImg), IPL_DEPTH_8U, 1);
	IplImage* B_Channel_Re = cvCreateImage(cvGetSize(leftImg), IPL_DEPTH_8U, 1);

	cvSplit(leftImg, B_Channel_le, G_Channel_le, R_Channel_le, 0);
	cvSplit(rightImg, B_Channel_ri, G_Channel_ri, R_Channel_ri, 0);
	cvSplit(midImg, B_Channel_mi, G_Channel_mi, R_Channel_mi, 0);

	cout << " 5번 각각 color space 나누기 완료~ (Iplimage)" << endl;

	// 열린 두장의 디스패리티 & 세장의 원본 - disparity_img_L_M disparity_img_M_R leftImg midImg rightImg
	// 이제 view interpolation을 시작한다. 
	result = cvCreateImage(cvSize(leftImg->width, leftImg->height), IPL_DEPTH_8U, 3);
	cvSetZero(R_Channel_Re);
	cvSetZero(G_Channel_Re);
	cvSetZero(B_Channel_Re);

	for (n = 0; n < NUM_OF_INTERPOLATIED_IMAGE; ++n) // NUM_OF_INTERPOLATIED_IMAGE는 항상 짝수일 경우만 생각. 
	{	
		if (n >= NUM_OF_INTERPOLATIED_IMAGE / 2)// right & middle 
		{
			for (c = 0; c < leftImg->height; ++c)
			{
				for (r = 0; r < leftImg->width; ++r)
				{
					if ((r - (disparity_img_M_R[c*width + r] * (9 - n) / 10)) > 0){
						R_Channel_Re->imageData[c * leftImg->width + r - (disparity_img_M_R[c*width + r] * (9 - n) / 10)] = R_Channel_ri->imageData[c * leftImg->width + r];
						G_Channel_Re->imageData[c * leftImg->width + r - (disparity_img_M_R[c*width + r] * (9 - n) / 10)] = G_Channel_ri->imageData[c * leftImg->width + r];
						B_Channel_Re->imageData[c * leftImg->width + r - (disparity_img_M_R[c*width + r] * (9 - n) / 10)] = B_Channel_ri->imageData[c * leftImg->width + r];
					}
				}
			}
		}
		else// middle & left 순서
		{
			for (c = 0; c < leftImg->height; ++c)
			{
				for (r = 0; r < leftImg->width; ++r)
				{
					if ((r - (disparity_img_L_M[c*width + r] * (9 - n) / 10)) > 0){
						R_Channel_Re->imageData[c * leftImg->width + r - (disparity_img_L_M[c*width + r] * (9 - n) / 10)] = R_Channel_mi->imageData[c * leftImg->width + r];
						G_Channel_Re->imageData[c * leftImg->width + r - (disparity_img_L_M[c*width + r] * (9 - n) / 10)] = G_Channel_mi->imageData[c * leftImg->width + r];
						B_Channel_Re->imageData[c * leftImg->width + r - (disparity_img_L_M[c*width + r] * (9 - n) / 10)] = B_Channel_mi->imageData[c * leftImg->width + r];
					}
					else{//이론적으로 여기는 채울수가 없다. 젠장 
						if (c > 0){
							R_Channel_Re->imageData[c * leftImg->width + r - (disparity_img_L_M[c*width + r] * (9 - n) / 10)] = 255;// R_Channel_mi->imageData[(c - 1) * leftImg->width + width + (disparity_img_M_R[c*width + r] * (9 - n) / 10)];
							G_Channel_Re->imageData[c * leftImg->width + r - (disparity_img_L_M[c*width + r] * (9 - n) / 10)] = 255;// G_Channel_mi->imageData[(c - 1) * leftImg->width + width + (disparity_img_M_R[c*width + r] * (9 - n) / 10)];
							B_Channel_Re->imageData[c * leftImg->width + r - (disparity_img_L_M[c*width + r] * (9 - n) / 10)] = 0;// B_Channel_mi->imageData[(c - 1) * leftImg->width + width + (disparity_img_M_R[c*width + r] * (9 - n) / 10)];
						}
					}
				}
			}
		}

		cvMerge(B_Channel_Re, G_Channel_Re, R_Channel_Re, NULL, result);
		cout << " 6번 view interpolation 완료 - " << n << endl;
		cvSaveImage(outcome[n], result);
		cvSetZero(R_Channel_Re);
		cvSetZero(G_Channel_Re);
		cvSetZero(B_Channel_Re);
		cvSetZero(result);
		
	}
	cout << " 7번 저장 완료(png)" << endl;

	/// 3-4. inpaint하여 영상의 hole을 매꾸어 줌. 
	/// 3-5. (필터를 이용하여 블러처리 - 옵션)

	///4. 처리완료한 영상 4장씩 8장 ( left - mid 사이 4장, mid - right 사이 4장 ) 전송(저장은 옵션)


	// test - 중간 결과물 및 완성 결과물 윈도우 만들어서 보여주기
	//cvNamedWindow("left original", 1); cvShowImage("left original", leftImg);
	/*cvNamedWindow("mid original", 1); cvShowImage("mid original", midImg);
	cvNamedWindow("right original", 1); cvShowImage("right original", rightImg);*/
	disparity_test = cvLoadImage(disp_l_m); disparity_test2 = cvLoadImage(disp_m_r);
	cvNamedWindow("1", 1); cvShowImage("1", disparity_test);
	cvNamedWindow("2", 1); cvShowImage("2", disparity_test2);

	while (1)
	{
		int c = cvWaitKey(10);
		if (c == 27)
			break;
	}


	///fin. release 
	cvReleaseImage(&leftImg);
	cvReleaseImage(&rightImg);
	cvReleaseImage(&midImg);
	cvReleaseImage(&result);
	
	cvReleaseImage(&R_Channel_le);
	cvReleaseImage(&G_Channel_le);
	cvReleaseImage(&B_Channel_le);

	cvReleaseImage(&R_Channel_mi);
	cvReleaseImage(&G_Channel_mi);
	cvReleaseImage(&B_Channel_mi);

	cvReleaseImage(&R_Channel_ri);
	cvReleaseImage(&G_Channel_ri);
	cvReleaseImage(&B_Channel_ri);

	cvReleaseImage(&R_Channel_Re);
	cvReleaseImage(&G_Channel_Re);
	cvReleaseImage(&B_Channel_Re);


	delete []disparity_img_L_M;
	delete []disparity_img_M_R;

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


/*$Id: qx_nonlocal_cost_aggregation.cpp,v 1.1 2007/02/16 04:11:12 liiton Exp $*/
#include "basic.h"
#include "nonlocal.h"
qx_nonlocal_cost_aggregation::qx_nonlocal_cost_aggregation()
{
	m_buf_u2 = NULL;
	m_buf_u3 = NULL;
	m_buf_f2 = NULL;
	m_buf_d3 = NULL;
}
qx_nonlocal_cost_aggregation::~qx_nonlocal_cost_aggregation()
{
	clean();
}
void qx_nonlocal_cost_aggregation::clean()
{
	qx_freeu_3(m_buf_u2); m_buf_u2 = NULL;
	qx_freeu_4(m_buf_u3); m_buf_u3 = NULL;
	qx_freef_3(m_buf_f2); m_buf_f2 = NULL;
	qx_freed_4(m_buf_d3); m_buf_d3 = NULL;
}
int qx_nonlocal_cost_aggregation::init(int h, int w, int nr_plane, double sigma_range, double max_color_difference, double max_gradient_difference, double weight_on_color)
{
	clean();
	m_h = h; m_w = w; m_nr_plane = nr_plane; m_sigma_range = sigma_range;
	m_max_color_difference = max_color_difference; m_max_gradient_difference = max_gradient_difference;
	m_weight_on_color = weight_on_color;
	m_weight_on_color_inv = 1 - m_weight_on_color;

	m_buf_f2 = qx_allocf_3(6, m_h, m_w);
	m_buf_d3 = qx_allocd_4(4, m_h, m_w, m_nr_plane);
	m_cost_vol = m_buf_d3[0];
	m_cost_vol_backup = m_buf_d3[1];
	m_cost_vol_temp = m_buf_d3[2];
	m_cost_vol_right = m_buf_d3[3];
	m_cost_min = m_buf_f2[0];
	m_cost_temp = m_buf_f2[1];
	m_cost = m_buf_f2[2];
	m_gradient_shifted = m_buf_f2[3];
	m_gradient_left = m_buf_f2[4];
	m_gradient_right = m_buf_f2[5];
	for (int y = 0; y<m_h; y++) for (int x = 0; x<m_w; x++) m_cost_temp[y][x] = QX_DEF_FLOAT_MAX;
	m_buf_u3 = qx_allocu_4(2, m_h, m_w, 3);
	m_image_shifted = m_buf_u3[0];
	m_image_temp = m_buf_u3[1];
	m_buf_u2 = qx_allocu_3(3, m_h, m_w);
	m_disparity = m_buf_u2[0];
	m_disparity_right = m_buf_u2[1];
	m_mask_occlusion = m_buf_u2[2];


	for (int i = 0; i<256; i++) m_table[i] = exp(-double(i) / (m_sigma_range * 255));
	m_tf.init(m_h, m_w, 3, m_sigma_range, 4);
	m_tf_right.init(m_h, m_w, 3, m_sigma_range, 4);
	return(0);
}
int qx_nonlocal_cost_aggregation::matching_cost(unsigned char***left, unsigned char***right)
{
	m_left = left; m_right = right;
	matching_cost_from_color_and_gradient(left, right);
	image_copy(m_cost_vol_backup, m_cost_vol, m_h, m_w, m_nr_plane);
	qx_stereo_flip_corr_vol(m_cost_vol_right, m_cost_vol, m_h, m_w, m_nr_plane);

	m_tf.build_tree(m_left[0][0]);
	m_tf_right.build_tree(m_right[0][0]);
	return(0);
}
int qx_nonlocal_cost_aggregation::disparity(unsigned char**disparity, bool use_nonlocal_post_processing)
{
	int radius = 2;
	image_copy(m_cost_vol, m_cost_vol_backup, m_h, m_w, m_nr_plane);
	m_tf.filter(m_cost_vol[0][0], m_cost_vol_temp[0][0], m_nr_plane);
	depth_best_cost(m_disparity, m_cost_vol, m_h, m_w, m_nr_plane);
	ctmf(m_disparity[0], disparity[0], m_w, m_h, m_w, m_w, radius, 1, m_h*m_w);
	//image_copy(disparity,m_disparity,m_h,m_w);

	if (use_nonlocal_post_processing)
	{
		image_copy(m_cost_vol, m_cost_vol_right, m_h, m_w, m_nr_plane);
		m_tf_right.filter(m_cost_vol[0][0], m_cost_vol_temp[0][0], m_nr_plane);
		depth_best_cost(m_disparity, m_cost_vol, m_h, m_w, m_nr_plane);
		ctmf(m_disparity[0], m_disparity_right[0], m_w, m_h, m_w, m_w, radius, 1, m_h*m_w);
		//image_display(m_disparity_right,m_h,m_w);
		//image_display(disparity,m_h,m_w);

		//qx_occlusion_solver_left_right(disparity,m_disparity_right,m_h,m_w,m_nr_plane,false);
		qx_detect_occlusion_left_right(m_mask_occlusion, disparity, m_disparity_right, m_h, m_w, m_nr_plane);
		image_zero(m_cost_vol, m_h, m_w, m_nr_plane);
		//int th=int(0.1*m_nr_plane+0.5);
		for (int y = 0; y<m_h; y++) for (int x = 0; x<m_w; x++) if (!m_mask_occlusion[y][x])
		{
			for (int d = 0; d<m_nr_plane; d++)
				//m_cost_vol[y][x][d]=min(abs(disparity[y][x]-d),th);
				//m_cost_vol[y][x][d]=(disparity[y][x]-d)*(disparity[y][x]-d);
				m_cost_vol[y][x][d] = abs(disparity[y][x] - d);
		}
		m_tf.update_table(m_sigma_range / 2);
		m_tf.filter(m_cost_vol[0][0], m_cost_vol_temp[0][0], m_nr_plane);
		//depth_best_cost(disparity,m_cost_vol,m_h,m_w,m_nr_plane);
		depth_best_cost(m_disparity, m_cost_vol, m_h, m_w, m_nr_plane);
		ctmf(m_disparity[0], disparity[0], m_w, m_h, m_w, m_w, radius, 1, m_h*m_w);

	}
	return(0);
}
void qx_nonlocal_cost_aggregation::matching_cost_from_color_and_gradient(unsigned char ***left, unsigned char ***right)
{
	image_zero(m_cost_vol, m_h, m_w, m_nr_plane);
	compute_gradient(m_gradient_left, left);
	compute_gradient(m_gradient_right, right);

	for (int i = 0; i<m_nr_plane; i++)
	{
		for (int y = 0; y<m_h; y++) //shift the right image by i pixels
		{
			image_copy((&m_image_shifted[y][i]), right[y], m_w - i, 3);
			memcpy(&(m_gradient_shifted[y][i]), m_gradient_right[y], sizeof(float)*(m_w - i));
			for (int x = 0; x<i; x++)
			{
				qx_memcpy_u3(m_image_shifted[y][x], right[y][0]);//m_cost_on_border_occlusion;
				m_gradient_shifted[y][x] = m_gradient_right[y][0];//m_cost_on_border_occlusion;
			}
		}
		//for(int y=0;y<m_h;y++) for(int x=0;x<m_w-1;x++) 
		for (int y = 0; y<m_h; y++) for (int x = 0; x<(m_w); x++)
		{
			double cost = 0;
			for (int c = 0; c<3; c++) cost += abs(left[y][x][c] - m_image_shifted[y][x][c]);
			cost = min(cost / 3, m_max_color_difference);
			double cost_gradient = min((double)abs(m_gradient_left[y][x] - m_gradient_shifted[y][x]), m_max_gradient_difference);
			m_cost_vol[y][x][i] = m_weight_on_color*cost + m_weight_on_color_inv*cost_gradient;
		}
	}
}
void qx_nonlocal_cost_aggregation::compute_gradient(float**gradient, unsigned char***image)
{
	float gray, gray_minus, gray_plus;
	for (int y = 0; y<m_h; y++)
	{
		gray_minus = rgb_2_gray(image[y][0]);
		gray = gray_plus = rgb_2_gray(image[y][1]);
		gradient[y][0] = gray_plus - gray_minus + 127.5;
		for (int x = 1; x<m_w - 1; x++)
		{
			gray_plus = rgb_2_gray(image[y][x + 1]);
			gradient[y][x] = 0.5*(gray_plus - gray_minus) + 127.5;
			gray_minus = gray;
			gray = gray_plus;
		}
		gradient[y][m_w - 1] = gray_plus - gray_minus + 127.5;
	}
}
