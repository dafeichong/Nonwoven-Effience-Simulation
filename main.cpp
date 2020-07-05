#include <QtCore/QCoreApplication>
#include <iostream>
#include <vector>
#include <opencv2/opencv.hpp>
#include <string>
#include <algorithm>

void read_images_from_file(const char* input_filename, std::vector<cv::Mat>& images, int num_of_images)
{
	//read images into vector<Mat>
	for (int a = 0; a < num_of_images; a++)
	{
		std::string name = input_filename + cv::format("%d.jpg", a);
		cv::Mat img = cv::imread(name, CV_LOAD_IMAGE_ANYDEPTH);
		if (img.empty())
		{
			std::cerr << "whaa " << name << " can't be loaded!" << std::endl;
			continue;
		}
		images.push_back(img);
	}
}

void Projection(std::vector<cv::Mat>& images, cv::Mat projection, int angle, int axis)
{

}
//������ά�ռ�㵽��ľ���
float point2point(int x1, int y1, int z1, int x2, int y2, int z2)
{
	int x_dist = x1 - x2;
	int y_dist = y1 - y2;
	int z_dist = z1 - z2;
	return sqrt(x_dist* x_dist + y_dist * y_dist + z_dist * z_dist);
}

/*
������ά���ƵĶ�άͶӰ��angle����Ϊ������axis == 0��������y����ת��axis == 1 ��������x����ת
*/
void Projection(const cv::Mat& depth_map, cv::Mat& projection, int angle, int axis)
{

}
/*
�ṩBresenhamCircle�����ڲ����ã������жϵ�ǰ���Ƿ���ͼ�����������ά����ײ
@return Parameter: true ����ǰ����ͼ������߱���ά��ס
*/
bool isBlock(const cv::Mat& mask, int x, int y, int x_offset, int y_offset)
{
	if (mask.at<uchar>(y + y_offset, x + x_offset) == 0)
		return true;
	else if (mask.at<uchar>(-y + y_offset, -x + x_offset) == 0)
		return true;
	else if (mask.at<uchar>(-y + y_offset, x + x_offset) == 0)
		return true;
	else if (mask.at<uchar>(y + y_offset, -x + x_offset) == 0)
		return true;
	else if (mask.at<uchar>(x + y_offset, y + x_offset) == 0)
		return true;
	else if (mask.at<uchar>(-x + y_offset, -y + x_offset) == 0)
		return true;
	else if (mask.at<uchar>(-x + y_offset, y + x_offset) == 0)
		return true;
	else if (mask.at<uchar>(x + y_offset, -y + x_offset) == 0)
		return true;
	else
		return false;
}
/*
Bresenham��Բ�㷨�޸İ�:���ڼ�ⲻͬ�뾶��Բ�Ƿ�����ά���ཻ
@Input Parameter mask : ��ά�ڰ�ͼ����ɫ255����׶�����ɫ0������ά��
@Iutput Parameter x_offset : x��ƽ����
@Iutput Parameter y_offset: y��ƽ����
@Input Parameter radius : ���˿����İ뾶��С
@Return Parameter : true �����������ײ�����ؿ����ɹ�
*/
bool BresenhamCircle(const cv::Mat& mask, int x_offset, int y_offset, int radius)
{
	int p = 3 - 2 * radius;
	int y = radius;
	for (int x = 0; x <= y; x++) 
	{
		//���������ײ���򷵻�true
		if (isBlock(mask, x, y, x_offset, y_offset))
			return true;
		if (p >= 0) 
		{
			p += 4 * (x - y) + 10;
			y--;
		}
		else 
		{
			p += 4 * x + 6;
		}
	}
	//�뾶Ϊradius��Բ�ϵĵ�û�б���ά��ײ����
	return false;
}


/*
���������͸��
@Input Parameter mask : ��ά�ڰ�ͼ����ɫ����׶�����ɫ������ά��
@InputParameter radius : ���˿����İ뾶��С
@Output Parameter count : ���ص���Ϊͨ���Ŀ�����
@Output Parameter effience: ���ص���Ϊ������͸��Ч�ʣ���Χ��[0,1]֮��
*/
void FilterEffience(const cv::Mat& mask, const int& radius, int& count, float& effience)
{
	using namespace std;
	int rows = mask.rows;
	int cols = mask.cols;
	count = 0;
	effience = 0.0;
	int base = (rows - 2 * radius) * (cols - 2 * radius);		//������count/base = effience

	for (int i = radius; i < rows - radius; i++)
	{
		for (int j = radius; j < cols - radius; j++)
		{
			//����ǿ׶�,�Ž��м���
			if (mask.at<uchar>(i, j) == 255)
			{
				//���û�б����أ�������ܳɹ�ͨ����count++
				if (!BresenhamCircle(mask, j, i, radius))
				{
					count++;
				}
			}
		}
	}
	effience = count * 1.0 / base * 1.0;
	cout << base << endl;
}
/*
����׾��ֲ�
@Input Parameter mask : ��ά�ڰ�ͼ����ɫ����׶�����ɫ������ά��
@Output Parameter distribution: ����Ϊ���п׵Ŀ׾���С�ֲ���������
*/
void PoreDistribution(const cv::Mat& mask, std::vector<int>& distribution)
{
	int rows = mask.rows;
	int cols = mask.cols;
	for (int i = 0; i < rows; i++)
	{
		for (int j = 0; j < cols; j++)
		{
			int max_radius = 1;
			if (mask.at<uchar>(i, j) == 255)
			{
				int radius = 1;
				while (BresenhamCircle(mask, j, i, radius))
				{
					max_radius = std::max(max_radius, radius);
					radius++;
				}
			}
		}
	}
}


int main(int argc, char *argv[])
{
	//�ı�����һ��pixelԼ����3um
	using namespace std;
    QCoreApplication a(argc, argv);
	cv::Mat mask = cv::imread("fused_image_crop.jpg", CV_LOAD_IMAGE_GRAYSCALE);
	//cv::resize(mask, mask, cv::Size(mask.cols * 2, mask.rows * 2));
	//cv::threshold(mask, mask, 128, 255, CV_THRESH_OTSU);
	cv::threshold(mask, mask, 30, 255, CV_THRESH_BINARY_INV);
	cv::imshow("1", mask);
	int count = 0;
	float effience = 0.0;
	int radius = 1;
	FilterEffience(mask, radius, count, effience);
	cout << "count:" << count << endl;
	cout << "effience:" << 1 - effience << endl;
    return a.exec();
}
