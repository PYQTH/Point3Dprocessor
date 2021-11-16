#include <iostream>
#include <fstream>

#include <string>
#include <string.h>
#include <vector>

using namespace std;


int main(int argc, char *argv[])
{
    // 目的：要得到的目标点的距离和坐标，都用二维向量表示
    vector<vector<double>> point(10,vector<double>(3));
    vector<vector<double>> distance(10,vector<double>(10));
  
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
   
    // 读取点的坐标（）不包含距离），格式：1.0,2.0,3.0
    for (int i = 0; i < POINT_COORDINATE_LINE; i++)
    {
        if (string_point_dist[i] != "")
        {
            //读取前中括号到后中括号之间的坐标数据，所以要减前面三个符号加上最后一个后中括号，一共减4
            int string_length = string_point_dist[i].length() - 4; 
            point_string.push_back(string_point_dist[i].substr(3, string_length)); 
        }
        
    }

    for (int i = POINT_COORDINATE_LINE; i < string_point_dist.size(); i++)
    {
        // 按格式读取点之间的距离，长度减掉数据前面四个字符（包括空格）
        int string_length = string_point_dist[i].length() - 4;
        distance_string.push_back(string_point_dist[i].substr(4, string_length));
    }
    

    // 可用于检验输出的 坐标值 和 点之间的相互 距离， 按顺序1-10，分别代表A-J
    // 距离按顺序依次代表
    for (int i = 0; i < point_string.size(); i++)
    {
        cout<<point_string[i]<<endl;
    }

    for (int i = 0; i < distance_string.size(); i++)
    {
        cout<<distance_string[i]<<endl;
    }

        
    for (int i = 0; i < point_string.size(); i++)
    {
        // string->char *
        char *s_input = (char *)point_string[2].c_str();
        const char * split = "，";
        // 以‘，’为分隔符拆分字符串
        char *p = strtok(s_input, split);
    }
    
    // string->char *
    char *s_input = (char *)point_string[2].c_str();
    const char * split = "，";
    // 以‘，’为分隔符拆分字符串
    char *p = strtok(s_input, split);
        
    
    // for (int i = 0; i < string_point_dist.size(); i++)
    // {
    //     for (int j = 0; j < string_point_dist[i].length(); j++)
    //     {
    //         point[i] = string_point_dist[i].substr(3, string_point_dist[i].length() - 2);
    //     }
        
    // }
    
    
    

    return 0;
}
