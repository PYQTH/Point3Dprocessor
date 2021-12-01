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
    //Use this two nested vector to describe ten points and distance of every two points, which is our final purpose
    vector<vector<double>> point(10,vector<double>(3));
    vector<vector<double>> distance_matrix(10,vector<double>(10));
  
    // 1. read the data from the file, including coordinate and ditance
    ifstream inPoint_Distance_data;
    string filename = "RawPointMeasurements.txt";

    inPoint_Distance_data.open(filename);
    if (inPoint_Distance_data.fail())
    {
        cout<<"Can not open the file, check the filename!!!";
    }else{
        cout<<"Data has read!"<<endl;
    }
    
    // 2.read the data to a string vector(string_point_dist) by line
    vector<string> string_point_dist;
    string line;
    
    while (getline(inPoint_Distance_data, line))
    {
        string_point_dist.push_back(line);
    }

    // 3.read the point coordinates data from the whole data in a needed format like this: 1.0,2.0,1.0
    // and the distance data, the format is just a number, like: 155.2
    vector<string> point_string;
    vector<string> distance_string;
    int const POINT_COORDINATE_LINE = 13; //how many lines of point coordinate the file has

    // read the point coordinate (not including distance data), and delete "[]",
    // and the data format is like this:1.0,2.0,3.0,is a kind of string and include ","
    for (int i = 0; i < POINT_COORDINATE_LINE; i++)
    {
        if (string_point_dist[i].size() > 3)
        {
            // read the coordinate data between "[" and "]",so we need to read data from index 3, and delete the last ']', so the total length is size - 4
            int string_length = string_point_dist[i].length() - 4; 
            point_string.push_back(string_point_dist[i].substr(3, string_length)); 
        }
    }
    // read the distance of every two points, only contain numbers, but the results sare still string
    for (int i = POINT_COORDINATE_LINE; i < string_point_dist.size(); i++)
    {
  
        // read the data in a speciofic format, which need to delete the first 4 symbols
        int string_length = string_point_dist[i].length() - 4;
        distance_string.push_back(string_point_dist[i].substr(4, string_length));
    }

    // delete the ',' 
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
    // delete '*' in the data
    for (int i = 0; i < point_temp.size(); i++)
    {
        for (int j = 0; j < point_temp[i].length(); j++)
        {
            point_temp[i].erase(remove(point_temp[i].begin(), point_temp[i].end(), '*'), point_temp[i].end());
        }
        
    }
    // make string data to double type data, and use a 2D vector to store it
    vector<vector<string>> point_temp_sec(10, vector<string>(3));
    int point_count = 0;
    for (int i = 0; i < point_temp_sec.size(); i++)
    {
        for (int j = 0; j < point_temp_sec[i].size(); j++)
        {
            point_temp_sec[i][j] = point_temp[point_count];
            point_count++;
            point[i][j] = stod(point_temp_sec[i][j]);
        }
    }

    // 4.Get the distance_matrix from the reading data distance_string
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
    // because there are two different points I,J, which format are not the same with formal ones
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
                distance_count++;
            }else{
                continue;
            }
            
        }
    }
    // because distance matrix is symmetric matrix, so fill it like this
    for (int i = 0; i < distance_matrix.size(); i++)
    {
        for (int j = 0; j < i; j++)
        {
            double tmp = distance_matrix[i][j];
            distance_matrix[j][i] = tmp;
        }
        
    }


    // Computation:
    // 1.get the origin distance matrix "distMatrix_origin" by using the coordinates of each points
    vector<vector<double>> distMatrix_origin(10, vector<double>(10));
    distMatrix_origin = get_distMatrix_from_ponits(point, distMatrix_origin.size());
    
    // 2.get the error of distance_matrix(measured) and distMatrix(computed),and name it lowest_error
    double lowestError = 0;
    lowestError = get_error_from_twoMarix(distance_matrix, distMatrix_origin);

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
                    // because there are some fixed value(*0.0) in points A,B,C,D, so we need to process them differently
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
    normal_distribution<double> random_number(0, scale);

    for (int i = 0; i < point.size(); i++)
    {
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
    normal_distribution<double> random_number(0, scale);

    for (int i = 0; i < point.size(); i++)
    {
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

