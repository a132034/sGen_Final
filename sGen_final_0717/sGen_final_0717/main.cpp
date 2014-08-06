/********************************************************************
*																	*
*	sGen 4th project - 'see real'									*
*	�ȼ���, ���¿�, ������, �ڵ���, �̼ҿ�, �̰��, �����			*
*	3���� ������ �̿��Ͽ� perspective view�� �����ϴ� ���α׷�		*
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
#include	"nonlocal.h"
#define QX_DEF_SIGMA							0.1//0.1
#define NUM_OF_INTERPOLATIED_IMAGE				8
#define MAX_DISPARITY							20

using namespace std;
void runStereo(char*filename_disparity_map, char*filename_left_image, char*filename_right_image, int max_disparity, bool use_post_processing);

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
	char *original_left							=	"TL.jpg";
	char *original_mid							=	"TR.jpg";
	char *original_right						=	"TL.jpg";
	char *disp_l_m								=	"disp_l_m.pgm";
	char *disp_m_r								=	"disp_m_r.pgm";
	char *stereo_left							=	"1.ppm";
	char *stereo_mid							=	"6.ppm";
	char *stereo_right							=	"11.ppm";
	char *outcome[NUM_OF_INTERPOLATIED_IMAGE]	=	 { "r2.png", "r3.png", "r4.png", "r5.png", "r7.png", "r8.png", "r9.png", "r10.png" };
	char *outLeft								=	"1.png";
	char *outMid								=	"6.png";
	char *outRight								=	"11.png";

	//3�� �ε� 
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

	cout << " 1�� �̹��� ���� �Ϸ�!!!! (���� �̹��� - IplImage) " << endl;
	//���� �̹��� ����
	cvSaveImage(stereo_left, leftImg);
	cvSaveImage(stereo_mid, midImg);
	cvSaveImage(stereo_right, rightImg);
	cvSaveImage(outLeft, leftImg);
	cvSaveImage(outMid, midImg);
	cvSaveImage(outRight, rightImg);
	cout << " 2�� �̹��� ���� �Ϸ�!!!! (���� �̹��� - jpg) " << endl;

	//������ ������ �ٽ� �ҷ��ͼ� stereomatching (rectify ���� �켱 ����)
	

	runStereo(disp_l_m, stereo_left, stereo_mid, MAX_DISPARITY, true);//including non-local post processing
	runStereo(disp_m_r, stereo_mid, stereo_right, MAX_DISPARITY, true);//including non-local post processing
	cout << " 3�� ���׷��� ��Ī �Ϸ�!!! (���и�Ƽ 2�� ����! - ppm) " << endl;

	/// 3-3. disparity map�� raw data�� ������ view interpolation(backward projection)
	// �ٽ� ����� ���и�Ƽ ���� ������(�̰� �ι� �ϸ� ��. 	
	disparity_img_L_M = ReadPgm(&height, &width, disp_l_m);
	disparity_img_M_R = ReadPgm(&height, &width, disp_m_r); 
	cout << " 4�� ���и�Ƽ ���� �Ϸ�!!!(ppm -> byte)" << endl;

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

	cout << " 5�� ���� color space ������ �Ϸ�~ (Iplimage)" << endl;

	// ���� ������ ���и�Ƽ & ������ ���� - disparity_img_L_M disparity_img_M_R leftImg midImg rightImg
	// ���� view interpolation�� �����Ѵ�. 
	result = cvCreateImage(cvSize(leftImg->width, leftImg->height), IPL_DEPTH_8U, 3);
	cvSetZero(R_Channel_Re);
	cvSetZero(G_Channel_Re);
	cvSetZero(B_Channel_Re);
	for (n = 0; n < NUM_OF_INTERPOLATIED_IMAGE; ++n) // NUM_OF_INTERPOLATIED_IMAGE�� �׻� ¦���� ��츸 ����. 
	{

		if (n < NUM_OF_INTERPOLATIED_IMAGE / 2)
		{
			for (c = 0; c < leftImg->height; ++c)
				{
				for (r = 0; r < leftImg->width; ++r)
					{
						if ((r - disparity_img_L_M[c*width + r] * (9 - n) / 10) >= 0)
						{
							if ((r - (disparity_img_L_M[c*width + r])) <= leftImg->width)
							{
								R_Channel_Re->imageData[c * leftImg->width + r - (disparity_img_L_M[c*width + r] * (9 - n) / 10)] = R_Channel_mi->imageData[c * leftImg->width + r - (disparity_img_L_M[c*width + r] * (9 - n) / 10)];
								G_Channel_Re->imageData[c * leftImg->width + r - (disparity_img_L_M[c*width + r] * (9 - n) / 10)] = G_Channel_mi->imageData[c * leftImg->width + r - (disparity_img_L_M[c*width + r] * (9 - n) / 10)];
								B_Channel_Re->imageData[c * leftImg->width + r - (disparity_img_L_M[c*width + r] * (9 - n) / 10)] = B_Channel_mi->imageData[c * leftImg->width + r - (disparity_img_L_M[c*width + r] * (9 - n) / 10)];
							}
							else
							{
								R_Channel_Re->imageData[c * leftImg->width + r - (disparity_img_L_M[c*width + r] * (9 - n) / 10)] = (R_Channel_le->imageData[c * leftImg->width + r]);// +R_Channel_ri->imageData[c * le->width + r - disp1[c*row + r]]) / 2; // B
								G_Channel_Re->imageData[c * leftImg->width + r - (disparity_img_L_M[c*width + r] * (9 - n) / 10)] = (G_Channel_le->imageData[c * leftImg->width + r]);// +G_Channel_ri->imageData[c * le->width + r - disp1[c*row + r]]) / 2; // G
								B_Channel_Re->imageData[c * leftImg->width + r - (disparity_img_L_M[c*width + r] * (9 - n) / 10)] = (B_Channel_le->imageData[c * leftImg->width + r]);// +B_Channel_ri->imageData[c * le->width + r - disp1[c*row + r]]) / 2; // R
							}
						}// �״ϱ� 1��. �� �ε����� 0���� ũ�� �ϴ� ó���� ��. �׸��� 0���� ū�� �������� ũ�� �ȵǴϱ� �װź��� ū�Ŵ� �� �ȵ�
						//�׸��� 2��. 0���ٵ� ������ �̰� ��... ��¿�����ֳ�? �����ʳ� �׳� ����ó�� �ع��� 
						else
						{
							//R_Channel_Re->imageData[c * leftImg->width + r - (disparity_img_L_M[c*width + r] * (9 - n) / 10)] = 0;//R_Channel_le->imageData[c * le->width + r];
							//G_Channel_Re->imageData[c * leftImg->width + r - (disparity_img_L_M[c*width + r] * (9 - n) / 10)] = 0;// G_Channel_le->imageData[c * le->width + r];
							//B_Channel_Re->imageData[c * leftImg->width + r - (disparity_img_L_M[c*width + r] * (9 - n) / 10)] = 255;// B_Channel_le->imageData[c * le->width + r];
						}
					}
				}
		}
		//else
		//{
		//	for (c = 0; c < leftImg->height; ++c)
		//	{
		//		for (r = 0; r < leftImg->width; ++r)
		//		{
		//			if ((r - disparity_img_M_R[c*width + r] * (9 - n) / 10) >= 0)
		//			{
		//				R_Channel_Re->imageData[c * leftImg->width + r - (disparity_img_M_R[c*width + r] * (9 - n) / 10)] = (R_Channel_mi->imageData[c * leftImg->width + r]);// +R_Channel_ri->imageData[c * le->width + r - disp1[c*row + r]]) / 2; // B
		//				G_Channel_Re->imageData[c * leftImg->width + r - (disparity_img_M_R[c*width + r] * (9 - n) / 10)] = (G_Channel_mi->imageData[c * leftImg->width + r]);// +G_Channel_ri->imageData[c * le->width + r - disp1[c*row + r]]) / 2; // G
		//				B_Channel_Re->imageData[c * leftImg->width + r - (disparity_img_M_R[c*width + r] * (9 - n) / 10)] = (B_Channel_mi->imageData[c * leftImg->width + r]);// +B_Channel_ri->imageData[c * le->width + r - disp1[c*row + r]]) / 2; // R

		//			}
		//			else if ((r - (disparity_img_M_R[c*width + r])) <= leftImg->width)
		//			{
		//				R_Channel_Re->imageData[c * leftImg->width + r - (disparity_img_M_R[c*width + r] * (9 - n) / 10)] = R_Channel_ri->imageData[c * leftImg->width + r - (disparity_img_M_R[c*width + r] * (9 - n) / 10)];
		//				G_Channel_Re->imageData[c * leftImg->width + r - (disparity_img_M_R[c*width + r] * (9 - n) / 10)] = G_Channel_ri->imageData[c * leftImg->width + r - (disparity_img_M_R[c*width + r] * (9 - n) / 10)];
		//				B_Channel_Re->imageData[c * leftImg->width + r - (disparity_img_M_R[c*width + r] * (9 - n) / 10)] = B_Channel_ri->imageData[c * leftImg->width + r - (disparity_img_M_R[c*width + r] * (9 - n) / 10)];
		//			}
		//			else
		//			{
		//				R_Channel_Re->imageData[c * leftImg->width + r - (disparity_img_M_R[c*width + r] * (9 - n) / 10)] = 0;// R_Channel_le->imageData[c * le->width + r];
		//				G_Channel_Re->imageData[c * leftImg->width + r - (disparity_img_M_R[c*width + r] * (9 - n) / 10)] = 0;// G_Channel_le->imageData[c * le->width + r];
		//				B_Channel_Re->imageData[c * leftImg->width + r - (disparity_img_M_R[c*width + r] * (9 - n) / 10)] = 255;// B_Channel_le->imageData[c * le->width + r];
		//			}
		//		}
		//	}
		//}
	
		cvMerge(B_Channel_Re, G_Channel_Re, R_Channel_Re, NULL, result);
		cout << " 6�� view interpolation �Ϸ� - " << n << endl;
		cvSaveImage(outcome[n], result);
		cvSetZero(R_Channel_Re);
		cvSetZero(G_Channel_Re);
		cvSetZero(B_Channel_Re);
		cvSetZero(result);
		
	}
	cout << " 7�� ���� �Ϸ�(png)" << endl;

	/// 3-4. inpaint�Ͽ� ������ hole�� �Ųپ� ��. 
	/// 3-5. (���͸� �̿��Ͽ� ��ó�� - �ɼ�)

	///4. ó���Ϸ��� ���� 4�徿 8�� ( left - mid ���� 4��, mid - right ���� 4�� ) ����(������ �ɼ�)


	// test - �߰� ����� �� �ϼ� ����� ������ ���� �����ֱ�
	cvNamedWindow("left original", 1); cvShowImage("left original", leftImg);
	cvNamedWindow("mid original", 1); cvShowImage("mid original", midImg);
	cvNamedWindow("right original", 1); cvShowImage("right original", rightImg);
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