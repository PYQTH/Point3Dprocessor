#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <algorithm>
#include <cmath>
#include <random>
#include <chrono>
#include <thread>
#include <mutex>

using namespace std;


double get_pointDistance(vector<double> point_1, vector<double> point_2);
vector<double> get_radomPoint(vector<double> point, double scale);
vector<double> get_radomInitPoint(vector<double> point, double scale);
vector<vector<double>> get_distMatrix_from_ponits(vector<vector<double>> point, int point_number);
double get_error_from_twoMarix(vector<vector<double>> distMtrix_1, vector<vector<double>> distMtrix_2);
void doCalculation(unsigned t_count, vector<vector<double>> pts);
void printfinal(vector<vector<double>> distance_matrix, vector<vector<double>> point, long long point_number);
vector<vector<vector<double>>> read_data(string filename,  vector<char> pointName, int point_number);

vector<vector<double>> distance_matrix;
vector<vector<double>> point;
vector<vector<double>> distMatrix_origin(10, vector<double>(10));
double lowestError;
bool flag = false;
long long point_number = 0;
double scale = 20;

int main(int argc, char *argv[])
{
    // prepare data structure

    string fileName = "RawPointMeasurements.txt";
    vector<char> pointName = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J'};
    const int POINT_NUMBER = 10;

    point = read_data(fileName, pointName, POINT_NUMBER)[0];
    distance_matrix = read_data(fileName, pointName, POINT_NUMBER)[1];

    // 3D Points Processing
    // 1.generate distances between know points，get a new distance matrix
    distMatrix_origin = get_distMatrix_from_ponits(point, distMatrix_origin.size());

    // 2.sum up the absolute differences between the new distance matrix and the measured one
    lowestError = 0;
    lowestError = get_error_from_twoMarix(distance_matrix, distMatrix_origin);
    // cout<<"lowesterror"<<lowestError<<endl;

    cout.precision(17);

    unsigned num_cpus = std::thread::hardware_concurrency();
    std::cout << "Launching " << num_cpus << " threads\n";

    std::mutex iomutex;
    std::vector<std::thread> threads(num_cpus);

    for (unsigned t_count = 0; t_count < num_cpus; ++t_count){
      threads[t_count] = thread([&iomutex, t_count]{
        {
          lock_guard<std::mutex> iolock(iomutex);
          cout << "Thread #" << t_count << ": on CPU " << sched_getcpu() << endl;
        }
        vector<vector<double>> pts(10,vector<double>(3));
        pts = point;
        doCalculation(t_count, pts);
      });
    }

    for (auto& t : threads) {
      t.join();
    }
    printfinal(distance_matrix, point, point_number);

    return 0;
}

void doCalculation(unsigned t_count, vector<vector<double>> pts){
  double local_error = lowestError;

  for( uint32_t itt = 0; true; itt++){
     bool improved = false;

     for (uint32_t itt2 = 0; itt2 < 1000; itt2++){
         point_number++;
         for (uint32_t i = 0; i < pts.size(); i++){
              if (i >= 4)
              {
                  pts[i] = get_radomPoint(point[i], scale);
              }else{
                  pts[i] = get_radomInitPoint(point[i], scale);
              }
          }

          double error = 0;
          error = get_error_from_twoMarix(distance_matrix, get_distMatrix_from_ponits(pts, 10));

          if (error < local_error){
              // pts = point_local; //not sure which point
              improved = true;
              local_error = error;

              if (local_error < lowestError){
                  point = pts;
                  lowestError = local_error;

              }else{
                  pts = point;
                  local_error = lowestError;
              }
          }
      }

      if (!improved){
          scale *= 0.999;
      }

      if ((itt % 100) == 0){
        cout<<"Current error " << t_count << "  "<< local_error<<" ("<<lowestError<<") "<<" Scale: "<<scale<<endl;
        if (scale < 0.01){
            flag = true;
            break;
        }
      }

      if (scale < 0.01){
          break;
      }

      if (local_error > 1.2*lowestError){
          pts = point;
          local_error = lowestError;
      }
    }
}

double get_pointDistance(vector<double> point_1, vector<double> point_2){
    double dist = pow(pow((point_1[0] - point_2[0]),2)
     + pow((point_1[1] - point_2[1]),2)
      + pow((point_1[2] - point_2[2]),2), 0.5);

    return dist;
}


vector<double> get_radomPoint(vector<double> point, double scale){
    double add_number;
    // default_random_engine random_engine;
    normal_distribution<double> random_number(0, scale);

    for (uint32_t i = 0; i < point.size(); i++){
        // epoch: nano secs since（1970/01/01 00:00:00 UTC）， will cut this value by unsigned data
        unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
        default_random_engine random_engine(seed);
        add_number = random_number(random_engine);
        point[i] += add_number;
    }
    return point;
}


vector<double> get_radomInitPoint(vector<double> point, double scale){
    double add_number;
    // default_random_engine random_engine;
    normal_distribution<double> random_number(0, scale);

    for (uint32_t i = 0; i < point.size(); i++){
        // epoch: nano secs since（1970/01/01 00:00:00 UTC）， will cut this value by unsigned data
        unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
        default_random_engine random_engine(seed);
        add_number = random_number(random_engine);
        if (point[i] != 0.0){
            point[i] += add_number;
        }
    }
    return point;
}


vector<vector<double>> get_distMatrix_from_ponits(vector<vector<double>> point, int point_number){
    vector<vector<double>> dist_matrix(point_number, vector<double>(point_number));
    for (uint32_t i = 0; i < point_number; i++){
        for (uint32_t j = i+1; j < point_number; j++){
            dist_matrix[i][j] = get_pointDistance(point[i], point[j]);
            dist_matrix[j][i] = dist_matrix[i][j]; // assign value to the matrix
        }
    }
    return dist_matrix;
}


double get_error_from_twoMarix(vector<vector<double>> distMtrix_1, vector<vector<double>> distMtrix_2){
    double error = 0;

    for (uint32_t i = 0; i < distMtrix_1.size(); i++){
        for (uint32_t j = i+1; j < distMtrix_1[i].size(); j++){
            error += abs(distMtrix_1[i][j] - distMtrix_2[i][j]);
            // cout<<error<<endl;
        }
    }
    return error;
}

void printfinal(vector<vector<double>> distance_matrix,
                vector<vector<double>> point, long long point_number){
  cout.precision(7);
  cout<<"\n\nOrigin Distance Matrix"<<endl;
  for (uint32_t i = 0; i < distance_matrix.size(); i++){
      for (uint32_t j = 0; j < distance_matrix[i].size(); j++){
          cout<<distance_matrix[i][j]<<"\t";
      }
      cout<<endl;
  }

  cout<<"\n\nComputed Distance Matrix"<<endl;
  vector<vector<double>> distMatrix_final(10, vector<double>(10));
  distMatrix_final = get_distMatrix_from_ponits(point, 10);
  for (uint32_t i = 0; i < distMatrix_final.size(); i++){
      for (uint32_t j = 0; j < distMatrix_final[i].size(); j++){
          cout<<distMatrix_final[i][j]<<"\t";
      }
      cout<<endl;
  }

  cout<<"\n";
  vector<string> P;
  P = {"A", "B", "C", "D", "E", "F", "G", "H", "I", "J"};
  cout<<"Final points (with constraints)"<<endl;
  for (uint32_t i = 0; i < point.size(); i++){
      cout << P[i] << ": [" << point[i][0] << ", "<< point[i][1] << ", "<< point[i][2] <<"]"<< endl;
  }

  double maxAbsError = 0;
  for (uint32_t i=0; i<distance_matrix.size(); i++){
      for (uint32_t j = 0; j < distance_matrix[i].size(); j++){
          maxAbsError = max(maxAbsError, abs(distance_matrix[i][j]- distMatrix_final[i][j]));
      }
  }
  cout<<"\n\nMax absolute error:  "<<maxAbsError<<endl;
  cout<<"calculation point number:"<<point_number<<endl;
}


vector<vector<vector<double>>> read_data(string filename, vector<char> pointName, int point_number)
{
    /**
    This function will return a vector with 2 elements, the first one is 
    the points coordinate, and the second one is the distance of every two 
    points.
    **/
    ifstream inPoint_Distance_data;
    string line, tmp;

    inPoint_Distance_data.open(filename);
    if (inPoint_Distance_data.fail()) {
        cout<<"The RawPointMeasurements.txt dose not exist!!!";
    };

    const int COORDINATE_DIMENSION = 3;
    const int DISTANCE_NUMBER = (point_number * (point_number-1)) / 2;
    vector<vector<double>> points(point_number, vector<double>(COORDINATE_DIMENSION));
    vector<vector<double>> distance_matrix(point_number,vector<double>(point_number));
    while (getline(inPoint_Distance_data, line))
    {
        for(uint32_t i = 0; i < point_number; i++ )
        {
            if(line[0] == pointName[i] && line[2] == '[')
            {
                // remove the first three chars, which aren't digits
                line = line.substr(3, line.length() - 3 );
                line.erase(remove(line.begin(), line.end(), '*'), line.end()); // remove *
                line.erase(remove(line.begin(), line.end(), ']'), line.end()); // remove [
                line.erase(remove(line.begin(), line.end(), ' '), line.end()); // remove space
                stringstream string_stream(line);
                uint32_t j = 0;
                while (string_stream.good())
                {
                    getline(string_stream, tmp, ',');
                    points[i][j] = stod(tmp);
                    j++;
                }
            }else{
                for(int32_t j = 0; j < DISTANCE_NUMBER; j++)
                {
                    if((line[0] == pointName[i] && line[2] == pointName[j]) || (line[2] == pointName[i] && line[0] == pointName[j]))
                    {
                        // remove the first three chars, which aren't digits
                        line = line.substr(3, line.length() - 3 );
                        line.erase(remove(line.begin(), line.end(), ' '), line.end()); // remove space
                        distance_matrix[j][i] = stod(line);
                        distance_matrix[i][j] = distance_matrix[j][i];
                    }
                }
            }
        }
    }
    vector<vector<vector<double>>> result_vector(2);
    result_vector[0] = points;
    result_vector[1] = distance_matrix;

    return result_vector;
}