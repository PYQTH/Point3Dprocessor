#include <iostream>
#include <math.h>
using namespace std;

// 参考的数据类型，包含已知点的坐标location，和所求的点到这个已知点坐标的距离dist
struct reference
{
    double location[3];
    double dist;
};

//函数声明
double *solve(reference refer_1, reference refer_2, reference refer_3);


int main(int argc, char *argv[])
{
    // 已知点的坐标
    double A[3] = {-250, 210, 0};
    double B[3] = {250, 210, 0};
    double C[3] = {-250, 0, 0};
    double D[3] = {250, 0, 0};

    // 所求的点到已知的4个点的距离
    double distance[6][4]={
        {654, 846.8, 689.2, 873},
        {1030, 855.5, 1053, 889.1},
        {830, 989, 805, 967.2},
        {1046, 879.3, 1025, 855.5},
        {940.6, 994.6, 916.8, 971.1},
        {1015.6, 957.6, 993.4, 935.4}
    };

    // 参考的数据类型，一共有6个点，每个点包含分别与4个已知点的 距离，以及 已知点的坐标，所以用6x4的2维数组表示
    reference refer[6][4];
    for (int i = 0; i < 6; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            refer[i][j].dist = distance[i][j];
            if (j == 0)
            {
                refer[i][j].location[0] = A[0];
                refer[i][j].location[1] = A[1];
                refer[i][j].location[2] = A[2];

            }else if (j == 1)
            {
                refer[i][j].location[0] = B[0];
                refer[i][j].location[1] = B[1];
                refer[i][j].location[2] = B[2];
            }else if (j == 2)
            {
                refer[i][j].location[0] = C[0];
                refer[i][j].location[1] = C[1];
                refer[i][j].location[2] = C[2];
            }else if (j == 3)
            {
                refer[i][j].location[0] = D[0];
                refer[i][j].location[1] = D[1];
                refer[i][j].location[2] = D[2];
            }else{
                refer[i][j].location[0] = 0;
                refer[i][j].location[1] = 0;
                refer[i][j].location[2] = 0;
            }

        }
        
    }

    // 4种情况的组合方式
    int combo[4][3] = {
        {0, 1, 2}, 
        {0, 1, 3}, 
        {0, 2, 3}, 
        {1, 2, 3}
    };

    //分别求每个点的在4种组合下的坐标，一共得到6x4=24个坐标，每个坐标是3维的，所以用一个6x4x3的3维数组表示
    double point_location[6][4][3];
    for (int i = 0; i < 6; i++)
    {
        for (int j = 0; j < 4; j++)
        {            
            point_location[i][j][0] = solve(refer[i][combo[j][0]], refer[i][combo[j][1]], refer[i][combo[j][2]])[0];
            point_location[i][j][1] = solve(refer[i][combo[j][0]], refer[i][combo[j][1]], refer[i][combo[j][2]])[1];
            point_location[i][j][2] = solve(refer[i][combo[j][0]], refer[i][combo[j][1]], refer[i][combo[j][2]])[2];
        }
        
    }

    // 求每个点坐标的平均值,一共得到六个点的坐标，每个坐标是三维，所以用一个二维数组表示
    double average_location[6][3];
    for (int i = 0; i < 6; i++)
    {
        average_location[i][0] = (point_location[i][0][0] + point_location[i][1][0] + point_location[i][2][0] + point_location[i][3][0]) / 4;
        average_location[i][1] = (point_location[i][0][1] + point_location[i][1][1] + point_location[i][2][1] + point_location[i][3][1]) / 4;
        average_location[i][2] = (point_location[i][0][2] + point_location[i][1][2] + point_location[i][2][2] + point_location[i][3][2]) / 4;
    }

    // 6个点坐标的平均值, 从第一行开始依次为：E,F,G,H,I,J
    for (int i = 0; i < 6; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            cout<<average_location[i][j]<<" ";
        }
        cout<<endl;
        
    }
    
    return 0;
}


double* solve(reference refer_1, reference refer_2, reference refer_3)
{
    double A[3];
    double B[3];
    double C[3];
    for (int i = 0; i < 3; i++)
    {
        A[i] = refer_1.location[i];
        B[i] = refer_2.location[i];
        C[i] = refer_3.location[i];
    }
    
    double EA = refer_1.dist;
    double EB = refer_2.dist;
    double EC = refer_3.dist;
    // cout<<EA<<endl;

    double x = ((2 * (A[1] - C[1])) * (EB*EB - EA*EA + A[0]*A[0] + A[1]*A[1] - B[0]*B[0] - B[1]*B[1])
     - (EC*EC - EA*EA + A[0]*A[0] + A[1]*A[1] - C[0]*C[0] - C[1]*C[1]) * 2 * (A[1] - B[1]))
     / (4 * (A[0] - B[0]) * (A[1] - C[1]) - 4 * (A[0] - C[0]) * (A[1] - B[1]));

    double y = (EC*EC - EA*EA + A[0]*A[0] + A[1]*A[1] - C[0]*C[0] - C[1]*C[1] - 2 * x * (A[0] - C[0]))
     / (2 * (A[1] - C[1]));

    double z = pow((EA*EA - x*x + 2 * A[0] * x - A[0]*A[0] - y*y + 2 * A[1] * y - A[1]*A[1]), 0.5);

    double *point_location = new double[3];
    point_location[0] = x;
    point_location[1] = y;
    point_location[2] = z;

    return point_location;
}

