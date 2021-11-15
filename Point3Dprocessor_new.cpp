#include <iostream>
#include <fstream>

#include <string>
#include <vector>

using namespace std;


int main(int argc, char *argv[])
{
    // double points[10][3];
    // 要得到的目标点的距离和坐标，都用二维向量表示
    vector<vector<double>> point(10,vector<double>(3));
    vector<vector<double>> distance(10,vector<double>(10));

    // for (int i = 0; i < 10; i++)
    // {
    //     cout<<points[i][2];
    // }
    
    ifstream inPoint_Distance_data;
    inPoint_Distance_data.open("RawPointMeasurements.txt");

    if (inPoint_Distance_data.fail())
    {
        cout<<"Can not open the fail, check the name!";
    }else{
        cout<<"Data has read!"<<endl;
    }
    
    vector<string> string_point_dist;
    string line;

    // read each lines to the varible,读取每一行
    int point_number = 0;
    while (getline(inPoint_Distance_data, line))
    {
        string_point_dist.push_back(line);
        point_number++;
    }
    
    // while (in_file)
    // {
    //     /* code */
    // }
    
    
    // for (int lineNumber = 0; lineNumber < string_point_dist.size(); lineNumber++)
    // {
    //     cout<<"-"<<string_point_dist[lineNumber]<<endl;
    //     cout<<lineNumber<<endl;
    //     lineNumber++;
    // }

    // read the point coordinates data from the whole data in a needed format like this: 1.0,2.0,1.0
    // cout<<string_point[0][1];
    int const POINT_COORDINATE_LINE = 13; //点的坐标占据的行数
    vector<string> point_string;
    for (int i = 0, j = 0; i < POINT_COORDINATE_LINE; i++)
    {
        if (string_point_dist[i] != "")
        {
            // point_string[j] = string_point_dist[i].substr(3, string_point_dist[i].length()-2); //读取前中括号到后中括号之间的坐标数据，所以要减2
            j++;
            cout<<point_string[j]<<" "<<j<<endl;
            cout<<j<<endl;
        }
        
    }
    
    // for (int i = 0; i < string_point_dist.size(); i++)
    // {
    //     for (int j = 0; j < string_point_dist[i].length(); j++)
    //     {
    //         point[i] = string_point_dist[i].substr(3, string_point_dist[i].length() - 2);
    //     }
        
    // }
    
    
    

    return 0;
}
