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
//计算三维空间点到点的距离
float point2point(int x1, int y1, int z1, int x2, int y2, int z2)
{
	int x_dist = x1 - x2;
	int y_dist = y1 - y2;
	int z_dist = z1 - z2;
	return sqrt(x_dist* x_dist + y_dist * y_dist + z_dist * z_dist);
}

/*
计算三维点云的二维投影，angle可以为负数，axis == 0代表是绕y轴旋转，axis == 1 代表是绕x轴旋转
*/
void Projection(const cv::Mat& depth_map, cv::Mat& projection, int angle, int axis)
{

}
/*
提供BresenhamCircle函数内部调用，用于判断当前点是否在图像外或者与纤维体碰撞
@return Parameter: true 代表当前点在图像外或者被纤维挡住
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
Bresenham画圆算法修改版:用于检测不同半径的圆是否与纤维体相交
@Input Parameter mask : 二维黑白图，白色255代表孔洞，黑色0代表纤维体
@Iutput Parameter x_offset : x的平移量
@Iutput Parameter y_offset: y的平移量
@Input Parameter radius : 过滤颗粒的半径大小
@Return Parameter : true 代表产生了碰撞，拦截颗粒成功
*/
bool BresenhamCircle(const cv::Mat& mask, int x_offset, int y_offset, int radius)
{
	int p = 3 - 2 * radius;
	int y = radius;
	for (int x = 0; x <= y; x++) 
	{
		//如果产生碰撞，则返回true
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
	//半径为radius的圆上的点没有被纤维碰撞拦截
	return false;
}


/*
计算颗粒穿透率
@Input Parameter mask : 二维黑白图，白色代表孔洞，黑色代表纤维体
@InputParameter radius : 过滤颗粒的半径大小
@Output Parameter count : 返回的数为通过的颗粒数
@Output Parameter effience: 返回的数为颗粒穿透率效率，范围在[0,1]之间
*/
void FilterEffience(const cv::Mat& mask, const int& radius, int& count, float& effience)
{
	using namespace std;
	int rows = mask.rows;
	int cols = mask.cols;
	count = 0;
	effience = 0.0;
	int base = (rows - 2 * radius) * (cols - 2 * radius);		//基数，count/base = effience

	for (int i = radius; i < rows - radius; i++)
	{
		for (int j = radius; j < cols - radius; j++)
		{
			//如果是孔洞,才进行计算
			if (mask.at<uchar>(i, j) == 255)
			{
				//如果没有被拦截，则颗粒能成功通过，count++
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
计算孔径分布
@Input Parameter mask : 二维黑白图，白色代表孔洞，黑色代表纤维体
@Output Parameter distribution: 返回为所有孔的孔径大小分布（已排序）
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
	//四倍镜下一个pixel约等于3um
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
