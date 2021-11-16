#include <iostream>
#include <fstream>

#include <string>
#include <string.h>
#include <vector>
#include <sstream>
#include <algorithm>

using namespace std;

vector<string> subStrToVec(string str,char sep)
{
    vector<string> vecArr;
    int flagSub = 0;
    for(int i=0;i<str.length();i++){
        if(str[i] == sep){
            string temp = str.substr(flagSub,i-flagSub);
            vecArr.push_back(temp);
            flagSub = i+1;
        }else if (i = str.length() - 1)
        {
            string temp = str.substr(flagSub,i-flagSub);
            vecArr.push_back(temp);
        }
        
    }
    return vecArr;
}



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
        cout<<"Can not open the fail, check the filename!";
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
    // for (int lineNumber = 0; lineNumber < string_point_dist.size(); lineNumber++)
    // {
    //     cout<<"-"<<string_point_dist[lineNumber]<<endl;
    //     cout<<lineNumber<<endl;
    //     lineNumber++;
    // }

    // 3.read the point coordinates data from the whole data in a needed format like this: 1.0,2.0,1.0
    // and the distance data, the format is just a number, like: 155.2
    vector<string> point_string;
    vector<string> distance_string;
    int const POINT_COORDINATE_LINE = 13; //点的坐标占据的前多少行
   
    // 读取点的坐标（不包含距离），删掉 中括号[]，格式：1.0,2.0,3.0，是包含  逗号, 的字符串
    for (int i = 0; i < POINT_COORDINATE_LINE; i++)
    {
        if (string_point_dist[i] != "")
        {
            //读取前中括号到后中括号之间的坐标数据，所以要减前面三个符号加上最后一个后中括号，一共减4
            int string_length = string_point_dist[i].length() - 4; 
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
            cout<<point[i][j]<<" ";
        }
        cout<<endl;
        // cout<<point_temp[i]<<endl;
    }


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

    for (int i = 0; i < distance_matrix.size(); i++)
    {
        for (int j = 0; j < distance_matrix[i].size(); j++)
        {
            cout<<distance_matrix[i][j]<<"\t";
        }
        cout<<endl;
        
    }
    
    
    return 0;
}
