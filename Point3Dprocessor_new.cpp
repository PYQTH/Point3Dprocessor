#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <algorithm>
#include <cmath>
#include <random>
#include <chrono>
#include <pthread.h>

using namespace std;


double get_pointDistance(vector<double> point_1, vector<double> point_2);
vector<double> get_radomPoint(vector<double> point, double scale);
vector<double> get_radomInitPoint(vector<double> point, double scale);
vector<vector<double>> get_distMatrix_from_ponits(vector<vector<double>> point, int point_number);
double get_error_from_twoMarix(vector<vector<double>> distMtrix_1, vector<vector<double>> distMtrix_2);
vector<vector<double>> copy_points(vector<vector<double>> copied_points);


int main(int argc, char *argv[])
{
    // 目的：要得到的目标点的距离和坐标，都用二维向量表示
    vector<vector<double>> point(10,vector<double>(3));
    vector<vector<double>> distance_matrix(10,vector<double>(10));
  
    // 1.读入文件内的数据，包含 坐标 和 距离数据
    ifstream inPoint_Distance_data;
    string filename = "RawPointMeasurements.txt";

    inPoint_Distance_data.open(filename);
    if (inPoint_Distance_data.fail())
    {
        cout<<"Can not open the file, check the filename!!!";
    }else{
        cout<<"Data has read!"<<endl;
    }
    
    // 2.把文件中的数据按行读入到 字符串向量 string_point_dist 中
    vector<string> string_point_dist;
    string line;
    
    while (getline(inPoint_Distance_data, line))
    {
        string_point_dist.push_back(line);
    }
    
    // 可用于输出读入的点的 坐标 和 距离 数据
    for (int lineNumber = 0; lineNumber < string_point_dist.size(); lineNumber++)
    {
        cout<<"-"<<string_point_dist[lineNumber]<<endl;
        cout<<lineNumber<<endl;
    }

    // 3.read the point coordinates data from the whole data in a needed format like this: 1.0,2.0,1.0
    // and the distance data, the format is just a number, like: 155.2
    vector<string> point_string;
    vector<string> distance_string;
    int const POINT_COORDINATE_LINE = 13; //点的坐标占据的前多少行
   
    // 读取点的坐标（不包含距离），删掉 中括号[]，格式：1.0,2.0,3.0，是包含  逗号, 的字符串
    for (int i = 0; i < POINT_COORDINATE_LINE; i++)
    {
        if (string_point_dist[i].size() > 3)
        {
            //读取前中括号到后中括号之间的坐标数据，所以要减前面三个符号和最后一个后中括号，一共加4（由于更改后的文件导致多了一个换行符号，所以-5）
            int string_length = string_point_dist[i].length() - 5; 
            point_string.push_back(string_point_dist[i].substr(3, string_length)); 
        }
    }
    //读取各点之间的距离，仅包含数字，但是为string类型
    for (int i = POINT_COORDINATE_LINE; i < string_point_dist.size(); i++)
    {
        // 按格式读取点之间的距离，长度减掉数据前面四个字符（包括空格）
        int string_length = string_point_dist[i].length() - 4;
        distance_string.push_back(string_point_dist[i].substr(4, string_length));
    }

    // 可用于检验输出的 坐标值 和 点之间的相互 距离， 按顺序1-10，分别代表A-J，都是字符串类型
    // 距离按顺序依次代表
    // for (int i = 0; i < point_string.size(); i++)
    // {
    //     cout<<point_string[i]<<endl;
    // }

    // for (int i = 0; i < distance_string.size(); i++)
    // {
    //     cout<<distance_string[i]<<endl;
    // }

    //删除间隔符 逗号 ，读入点坐标数据到1一个向量中，因为前面是属于 string 类型，还包含了 ，
    vector<string> point_temp;
    for (int i = 0; i < point_string.size(); i++)
    {
        stringstream string_stream(point_string[i]);
        while (string_stream.good())
        {
            string a;
            getline(string_stream, a, ',');
            point_temp.push_back(a);

        }
    }
    // 删除原数据中的 * ,否则将影响将其转换为 double 数据类型
    for (int i = 0; i < point_temp.size(); i++)
    {
        for (int j = 0; j < point_temp[i].length(); j++)
        {
            point_temp[i].erase(remove(point_temp[i].begin(), point_temp[i].end(), '*'), point_temp[i].end());
        }
        
    }
    //完成点坐标数据的读入，将 double 类型的数据存入到最先定义的  二维向量 vector<vector<string>> point(10, vector<double>(3))中
    vector<vector<string>> point_temp_sec(10, vector<string>(3));
    int point_count = 0;
    for (int i = 0; i < point_temp_sec.size(); i++)
    {
        for (int j = 0; j < point_temp_sec[i].size(); j++)
        {
            point_temp_sec[i][j] = point_temp[point_count];
            point_count++;
            // cout<<point_temp_sec[i][j]<<" ";
            point[i][j] = stod(point_temp_sec[i][j]);
            // cout<<point[i][j]<<" ";
        }
        // cout<<endl;
        // cout<<point_temp[i]<<endl;
    }

    // 由于A,B,C,D带 * 的0保持不变,把A,B,C,D中的0元素转换为const常数
    // for (int i = 0; i < 4; i++)
    // {
    //     for (int j = 0; j < point[i].size(); j++)
    //     {
    //         if (point[i][j] == 0)
    //         {
    //             point[i][j] = (double const)0;
    //         }
            
    //     }
        
    // }
    


    // 4.根据 distance_string向量 读入并得到点的 距离矩阵 distance_matrix
    int distance_count = 0;
    const int KNOWN_POINT_NUMBER = 8;
    for (int i = 0; i < KNOWN_POINT_NUMBER; i++) //distance_string.size()-2
    {
        for (int j = i+1; j < KNOWN_POINT_NUMBER; j++)
        {
            distance_matrix[j][i] = stod(distance_string[distance_count]);
            distance_count++;
        }
        
    }
    // cout<<distance_count<<endl;
    //由于I、J 两个点到其他点的距离格式记录与前面8个点不同，所以需要特殊读入
    for (int i = KNOWN_POINT_NUMBER; i < KNOWN_POINT_NUMBER + 2; i++)
    {
        for (int j = 0; j < KNOWN_POINT_NUMBER+2; j++)
        {
            if (i != j )
            {
                if (i == KNOWN_POINT_NUMBER +1 && j == KNOWN_POINT_NUMBER)
                {
                    distance_matrix[i][j] = distance_matrix[j][i];
                    continue;
                }
                    distance_matrix[i][j] = stod(distance_string[distance_count]);
                // cout<<distance_string[distance_count]<<"---"<<endl;;
                distance_count++;
            }else{
                continue;
            }
            
        }
    }
    // 根据对称填满，A-J分别代表由0-9列和0-9行，与java代码里面同样含义
    for (int i = 0; i < distance_matrix.size(); i++)
    {
        for (int j = 0; j < i; j++)
        {
            double tmp = distance_matrix[i][j];
            distance_matrix[j][i] = tmp;
        }
        
    }


    //计算过程：
    // 1.根据所给的坐标计算所有点两两之间的距离，生成坐标点两两之间的距离矩阵
    vector<vector<double>> distMatrix_origin(10, vector<double>(10));
    distMatrix_origin = get_distMatrix_from_ponits(point, distMatrix_origin.size());
    
    // 2.计算所给的距离与计算距离的差值的绝对值的总和
    double lowestError = 0;
    lowestError = get_error_from_twoMarix(distance_matrix, distMatrix_origin);
    // cout<<"lowesterror"<<lowestError<<endl;

    bool flag = false;
    long long point_number = 0;
    for (int total_count = 0; true; total_count++)
    {
        vector<vector<double>> pts(10, vector<double>(3));
        pts = point;
        
        double scale = 20;
        double local_error = lowestError;

        for (int itt = 0; true; itt++)
        {
           bool improved = false;

           for (int itt2 = 0; itt2 < 1000; itt2++)
           {
               point_number++;
               for (int i = 0; i < pts.size(); i++)
                {
                    // pts[i] = get_radomPoint(point[i], scale);
                    if (i >= 4)
                    {
                        pts[i] = get_radomPoint(point[i], scale);
                    }else{
                        pts[i] = get_radomInitPoint(point[i], scale);
                    }
                                    
                }

                double error = 0;
                error = get_error_from_twoMarix(distance_matrix, get_distMatrix_from_ponits(pts, 10));

                if (error < local_error)
                {
                    // pts = point_local; //不确定是哪一个point
                    improved = true;
                    local_error = error;

                    if (local_error < lowestError)
                    {
                        point = pts;
                        lowestError = local_error;  

                    }else{
                        pts = point;
                        local_error = lowestError;
                    }
                
                }
        
            }

            if (!improved)
            {
                scale *= 0.999;
            }

            if ((total_count % 100) == 0)
            {
                cout<<"Current error: "<<local_error<<" ("<<lowestError<<") "<<endl;
                cout<<"Scale: "<<scale<<endl;
                if (scale < 0.01)
                {
                    flag = true;
                    break;
                }
                
            }
            
            if (local_error > 1.2*lowestError)
            {
                pts = point;
                local_error = lowestError;
            }
        
        }

        if (flag)
        {
            break;
        }     
    }


    cout<<"Final points (with constraints)"<<endl;
    for (int i = 0; i < point.size(); i++)
    {
        cout<<"[ ";
        for (int j = 0; j < point[i].size(); j++)
        {
            cout<<point[i][j]<<"\t";
        }
        cout<<" ]"<<endl;
        
    }

    cout<<"\n\nOrigin Distance Matrix"<<endl;
    for (int i = 0; i < distance_matrix.size(); i++)
    {
        for (int j = 0; j < distance_matrix[i].size(); j++)
        {
            cout<<distance_matrix[i][j]<<"\t";
        }
        cout<<endl;
    }
    
    cout<<"\n\nComputed Distance Matrix"<<endl;
    vector<vector<double>> distMatrix_final(10, vector<double>(10));
    distMatrix_final = get_distMatrix_from_ponits(point, 10);
    for (int i = 0; i < distMatrix_final.size(); i++)
    {
        for (int j = 0; j < distMatrix_final[i].size(); j++)
        {
            cout<<distMatrix_final[i][j]<<"\t";
        }
        cout<<endl;
    }

    double maxAbsError = 0;
    for (int i=0; i<distance_matrix.size(); i++)
    {
        for (int j = 0; j < distance_matrix[i].size(); j++)
        {
            maxAbsError = max(maxAbsError, abs(distance_matrix[i][j]- distMatrix_final[i][j]));
        }
        
    }
    cout<<"\n\nMax absolute error:  "<<maxAbsError<<endl;

    cout<<"calculation point number:"<<point_number<<endl;
    
    return 0;

}


double get_pointDistance(vector<double> point_1, vector<double> point_2)
{
    double dist = pow(pow((point_1[0] - point_2[0]),2)
     + pow((point_1[1] - point_2[1]),2)
      + pow((point_1[2] - point_2[2]),2), 0.5);
    
    return dist;
}


vector<double> get_radomPoint(vector<double> point, double scale)
{
    double add_number;
    // default_random_engine random_engine;
    normal_distribution<double> random_number(0, scale);

    for (int i = 0; i < point.size(); i++)
    {
        // 从epoch（1970年1月1日00:00:00 UTC）开始经过的纳秒数，unsigned类型会截断这个值
        unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
        default_random_engine random_engine(seed);
        add_number = random_number(random_engine); 
        point[i] += add_number;    
    }
       
    return point;
}


vector<double> get_radomInitPoint(vector<double> point, double scale)
{
    double add_number;
    // default_random_engine random_engine;
    normal_distribution<double> random_number(0, scale);

    for (int i = 0; i < point.size(); i++)
    {
        // 从epoch（1970年1月1日00:00:00 UTC）开始经过的纳秒数，unsigned类型会截断这个值
        unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
        default_random_engine random_engine(seed);
        add_number = random_number(random_engine); 
        if (point[i] != 0.0)
        {
            point[i] += add_number; 
        }
           
    }
       
    return point;
}


vector<vector<double>> get_distMatrix_from_ponits(vector<vector<double>> point, int point_number)
{
    vector<vector<double>> dist_matrix(point_number, vector<double>(point_number));

    for (int i = 0; i < point_number; i++)
    {
        for (int j = i+1; j < point_number; j++)
        {
            dist_matrix[i][j] = get_pointDistance(point[i], point[j]);
            dist_matrix[j][i] = dist_matrix[i][j]; //对阵矩阵赋值
        }
        
    }

    return dist_matrix;
}


double get_error_from_twoMarix(vector<vector<double>> distMtrix_1, vector<vector<double>> distMtrix_2)
{
    double error = 0;
    
    for (int i = 0; i < distMtrix_1.size(); i++)
    {
        for (int j = i+1; j < distMtrix_1[i].size(); j++)
        {
            error += abs(distMtrix_1[i][j] - distMtrix_2[i][j]);
            // cout<<error<<endl;
        }
    }

    return error;
}


vector<vector<double>> copy_points(vector<vector<double>> copied_points)
{
    vector<vector<double>> points(copied_points.size(),vector<double>(copied_points[0].size()));
    for (int i = 0; i < copied_points.size(); i++)
    {
        for (int j = 0; j < copied_points[i].size(); j++)
        {
            points[i][j] = copied_points[i][j];
        }
        
    }
    
    return points;
}

