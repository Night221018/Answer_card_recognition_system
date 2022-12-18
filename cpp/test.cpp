#include <opencv2/opencv.hpp>
#include <iostream>
#include <string.h>
#include <vector>

using namespace std;
using namespace cv;

string pathname = "D:\\Microsoft VS Code\\OpenCV\\cpp\\omr1.jpg";
Mat img, imgGray, imgBlur, imgCanny, imgContours, imgWarp, imgWarpGrade, imgThre, imgWarpGray, imgWarpGradeGray, imgGradeThre;
vector<Point> docPoint_max, docPoint_grade;
vector<Mat> boxes;

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

// 仿射变换
Mat getWarp(Mat image, vector<Point> points, float w, float h) {
    Mat imgWarp;
    Point2f src[4] = {points[0], points[1], points[2], points[3]};
    Point2f dst[4] = {{0.0f, 0.0f}, {w, 0.0f}, {0.0f, h}, {w, h}};
    Mat matrix = getPerspectiveTransform(src, dst);
    warpPerspective(image, imgWarp, matrix, Point(w, h));
    return imgWarp;
}

// 图像分割
vector<Mat> splitBox(Mat img) {
    vector<Mat> boxes;
    Mat img_cut, rio_img;
    int m = img.cols / 5;  // 宽度
    int n = img.rows / 5;  // 高度
    for (int i = 0; i < 5; ++i) {
        for (int j = 0; j < 5; ++j) {
            Rect rect(i * n, j * n, 60, 60);
            img_cut = Mat(img, rect);
            rio_img = img_cut.clone();
            boxes.push_back(rio_img);
        }
    }
    // for (int i = 0; i < boxes.size(); ++i) {
    //     string num = to_string(i);
    //     imshow(num, boxes[i]);
    // }
    return boxes;
}

// 答案展示
Mat showAnswer(Mat img, vector<int> index, vector<int> grading, vector<int> ans, int question, int choice) {
    int secW = img.cols / choice;
    int secH = img.rows / question;
    Scalar color;
    for (int i = 0; i < question; ++i) {
        int sx = (index[i] * secW) + (secW / 2);
        int sy = (i * secH) + (secH / 2);
        if (grading[i] == 1) {
            color = Scalar(0, 255, 0);
        } else {
            color = Scalar(0, 0, 255);
            circle(img, Point((ans[i] * secW) + (secW / 2), sy), 10, Scalar(0, 255, 0), FILLED);
        }
        circle(img, Point(sx, sy), 20, color, FILLED);
    }
    return img;
}

int main() {
    img = imread(pathname);

    resize(img, img, Size(500, 500));
    cvtColor(img, imgGray, COLOR_BGR2GRAY);
    GaussianBlur(imgGray, imgBlur, Size(5, 5), 0, 0);
    Canny(imgBlur, imgCanny, 50, 25, 3);

    imshow("img", img);
    imshow("imgGray", imgGray);
    imshow("imgBlur", imgBlur);
    imshow("imgCanny", imgCanny);

    // threshold(imgBlur, imgThre, 180, 255, THRESH_BINARY);
    // imshow("imgThre", imgThre);
    // Mat temp;
    // threshold(imgBlur, temp, 180, 255, THRESH_BINARY_INV);
    // imshow("temp", temp);

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

    /* 0.重新排序四个角点 */ 
    // drawPoints(img, cPoint[0], Scalar(0, 0, 200));
    // drawPoints(img, cPoint[1], Scalar(0, 0, 200));
    // imshow("img", img);
    docPoint_grade = reorder(cPoint[0]);
    docPoint_max = reorder(cPoint[1]);
    drawPoints(imgContours, docPoint_max, Scalar(0, 0, 200));
    drawPoints(imgContours, docPoint_grade, Scalar(0, 0, 200));
    imshow("imgContours", imgContours);

    /* 1.仿射变换 */
    imgWarp = getWarp(img, docPoint_max, 300, 300);
    imshow("imgWarp", imgWarp);

    imgWarpGrade = getWarp(img, docPoint_grade, 180, 100);
    imshow("imgWarpGrade", imgWarpGrade);    

    /* 2.二值化处理 */
    cvtColor(imgWarp, imgWarpGray, COLOR_BGR2GRAY);
    threshold(imgWarpGray, imgThre, 180, 255, THRESH_BINARY_INV);
    imshow("imgThre", imgThre);
    cvtColor(imgWarpGrade, imgWarpGradeGray, COLOR_BGR2GRAY);
    threshold(imgWarpGradeGray, imgGradeThre, 180, 255, THRESH_BINARY_INV);
    imshow("imgGradeThre", imgGradeThre);

    /* 3.图形分割 */
    boxes = splitBox(imgThre);

    /* 4.计算答案下标，对比正确答案 */
    int totalPixels;
    int myPixelVal[5][5] = {0};
    for (int i = 0; i < 25; ++i) {
        totalPixels = countNonZero(boxes[i]);
        myPixelVal[i % 5][i / 5] = totalPixels;
    }
    for (int i = 0; i < 5; ++i) {
        for (int j = 0; j < 5; ++j) {
            cout << "[" << i << "," << j << "] " << myPixelVal[i][j] << " ";
        }
        cout << endl;
    }
    vector<vector<int>> brr;
    for (int i = 0; i < 5; ++i) {
        vector<int> arr;
        for (int j = 0; j < 5; ++j) {
            arr.push_back(myPixelVal[i][j]);
        }
        brr.push_back(arr);
    }
    vector<int> index;
    for (int i = 0; i < brr.size(); ++i) {
        auto location = max_element(brr[i].begin(), brr[i].end());
        index.push_back((int)(location - brr[i].begin()));
        cout << *location << " " << index[i] << endl;
    }
    vector<int> ans = {1, 2, 0, 1, 4};
    vector<int> grading;
    int rightnum = 0;
    for (int i = 0; i < 5; ++i) {
        if (ans[i] == index[i]) {
            ++rightnum;
            grading.push_back(1);
        } else {
            grading.push_back(0);
        }
        cout << grading[i] << endl;
    }
    int score = (double)rightnum / 5 * 100;
    cout << "score = " << score << endl; 


    /* 5.展示答案 */
    Mat imgAns = imgWarp.clone();
    imgAns = showAnswer(imgAns, index, grading, ans, 5, 5);
    imshow("imgAns", imgAns);


    /* 6.评分 */

    /* 7.反向仿射变换 */

    /* 8.从摄像头中读取，给答题卡评分 */
    

    waitKey();

    system("pause");
    return 0;
}