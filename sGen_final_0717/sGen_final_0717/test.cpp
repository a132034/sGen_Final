#include "opencv/cv.h"
#include "opencv/highgui.h"

using namespace std;

void ips()
{
	int i, j;

	IplImage *test;

	if ( (test = cvLoadImage("test.jpg", CV_LOAD_IMAGE_GRAYSCALE)) = NULL);

	for (i = 0; i < test->height; ++i)
	{
		for (j = 0; j < test->width; ++j)
		{
			if (j < test->width / 2)
				test->imageData[i* test->width + j] = 255;
			else
				test->imageData[i* test->width + j] = 0;
		}
	}
	cvSaveImage("result.jpg", test);

}


#pragma once
#include	"test.cpp"

class ip{

public:
	void do_ips();
};

void ip::do_ips()
{
	ips();
}



extern void ips();
class ip{
	public do_ips();
};