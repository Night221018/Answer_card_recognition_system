#include <opencv2/opencv.hpp>
#include <iostream>
#include <string.h>
#include <vector>

using namespace std;
using namespace cv;

string pathname = "D:\\Microsoft VS Code\\OpenCV\\cpp\\omr1.jpg";
Mat img, imgGray, imgBlur, imgCanny, imgContours;
vector<Point> docPoint_max, docPoint_grade;

vector<vector<Point>> getContours(Mat imgage) {
    // 四个角点，闭合轮廓
    // 怎样让涂卡区从众多矩形中脱颖而出，面积，周长
    // 两个ROI，1.涂卡区  2.成绩
    vector<vector<Point>> contours;
    vector<Vec4i> hierarchy;

    findContours(imgage, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_NONE);

    vector<vector<Point>> biggest;   // 涂卡区
    // vector<vector<Point>> greade;    // 成绩区
    vector<vector<Point>> recCon(contours.size());   // 轮廓的角点坐标
    int maxArea = 0;
    for (int i = 0; i < contours.size(); ++i) {
        // 遍历每一个轮廓
        double area = contourArea(contours[i], false);
        if (area > 300) {
            double peri = arcLength(contours[i], true);
            approxPolyDP(contours[i], recCon[i], 0.1 * peri, true);
            if (recCon[i].size() == 4 && area > maxArea) {
                // drawContours(img, recCon, i, Scalar(0, 255, 0));
                // imshow("drawContours", img);
                maxArea = area;
                biggest.push_back(recCon[i]);
            } 
            
        }
    }
    return biggest;
}

// 画点
void drawPoints(Mat image, vector<Point> cPoint, Scalar color) {
    for (int i = 0; i < cPoint.size(); ++i) {
        circle(image, cPoint[i], 5, color, FILLED);
        putText(image, to_string(i), cPoint[i], FONT_HERSHEY_PLAIN, 5, color, 3);
    }
    return ;
}

// 给点重新排序
vector<Point> reorder(vector<Point> points) {
    vector<Point> newPoints;
    vector<int> sumPoints, subPoints;
    for (int i = 0; i < points.size(); ++i) {
        sumPoints.push_back(points[i].x + points[i].y);
        subPoints.push_back(points[i].x - points[i].y);
    }
    newPoints.push_back(points[min_element(sumPoints.begin(), sumPoints.end()) - sumPoints.begin()]);
    newPoints.push_back(points[max_element(subPoints.begin(), subPoints.end()) - subPoints.begin()]);
    newPoints.push_back(points[min_element(subPoints.begin(), subPoints.end()) - subPoints.begin()]);
    newPoints.push_back(points[max_element(sumPoints.begin(), sumPoints.end()) - sumPoints.begin()]);

    return newPoints;
}

int main() {
    img = imread(pathname);

    resize(img, img, Size(400, 400));
    cvtColor(img, imgGray, COLOR_BGR2GRAY);
    GaussianBlur(imgGray, imgBlur, Size(5, 5), 0, 0);
    Canny(imgBlur, imgCanny, 50, 25, 3);

    imshow("img", img);
    imshow("imgGray", imgGray);
    imshow("imgBlur", imgBlur);
    imshow("imgCanny", imgCanny);

    vector<vector<Point>> contours;
    vector<vector<Point>> cPoint;
    vector<Vec4i> hierarchy;
    imgContours = img.clone();
    // findContours(imgCanny, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_NONE);
    // drawContours(imgContours, contours, -1, Scalar(0, 255, 0), 2);

    // 如何找到涂卡区的矩形轮廓
    cPoint = getContours(imgCanny);
    drawContours(imgContours, cPoint, -1, Scalar(0, 255, 0), 2);
    imshow("imgContours", imgContours);

    // 1.重新排序四个角点
    drawPoints(img, cPoint[0], Scalar(0, 0, 200));
    drawPoints(img, cPoint[1], Scalar(0, 0, 200));
    imshow("img", img);
    
    docPoint_grade = reorder(cPoint[0]);
    docPoint_max = reorder(cPoint[1]);
    drawPoints(imgContours, docPoint_max, Scalar(0, 0, 200));
    drawPoints(imgContours, docPoint_grade, Scalar(0, 0, 200));
    imshow("imgContours", imgContours);

    // 2.仿射变换

    // 3.图形分割

    // 4.计算答案下标，对比正确答案

    // 5.标记答案

    // 6.评分

    // 7.反向仿射变换

    // 8.从摄像头中读取，给答题卡评分
    

    waitKey();

    system("pause");
    return 0;
}