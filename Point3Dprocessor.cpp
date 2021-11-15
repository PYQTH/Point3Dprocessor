#include <iostream>
// #include <cmath.h>
#include <string>
#include <vector>
#include <algorithm>
#include <map>
#include <unordered_map>
#include <set>
#include <cmath>
#include <optional>
#include <mutex>
#include <cctype>

#include "rectangularvectors.h"
#include "exceptionhelper.h"
#include "stringhelper.h"
#include "ExecutionService.h"

using namespace std;


class Point3DProcessor
{
    public:
        
    
    private:
        static vector<string> labels;
        static vector<Point*> points;
        static vector<vector<double>> dm;

        static double lowestError;
        static AtomicLong *totalCounter;

        static void printDistanceMatrix(vector<vector<double>> &dm);

    static class Point
	{
	    public:
            std::wstring label;
            std::vector<bool> fixed;
            std::vector<double> X, XX;
            Random *random;

            virtual void acceptShiftedValues();
            virtual double distance(Point *pt);
            virtual void acceptShiftedValues();
		    virtual double distance(Point *pt);
            virtual double perturbedDistance(Point *pt);

            virtual Point *copyOf();

            virtual void copyPositionFrom(Point *pt);

            virtual std::wstring toString();



		virtual ~Point()
		{
			delete random;
		}

        Point::Point(const std::wstring &label, std::vector<double> &X, std::vector<bool> &fx)
            {
                this->label = label;
                this->X = X;
                this->fixed = fx;

                XX = std::vector<double>(3);
                XX[0] = X[0];
                XX[1] = X[1];
                XX[2] = X[2];

                random = new Random();
            }        
        
        void randomShift(double scale)
        {
            for (int i = 0; i < 3; i++)
            {
                XX[i] = X[i];
                if (fixed[i])
                {
                    continue;
                }

                XX[i] += random::nextGaussian() * scale;
            }
        }
 
        void acceptShiftedValues()
            {
                System::arraycopy(XX, 0, X, 0, 3);
            }

        double distance(Point *pt)
            {
                double dx = X[0] - pt->X[0];
                double dy = X[1] - pt->X[1];
                double dz = X[2] - pt->X[2];

                return sqrt(dx * dx + dy * dy + dz * dz);
            }

        double perturbedDistance(Point *pt)
            {
                double dx = XX[0] - pt->XX[0];
                double dy = XX[1] - pt->XX[1];
                double dz = XX[2] - pt->XX[2];

                return sqrt(dx * dx + dy * dy + dz * dz);
            }

        Point *copyOf()
            {
                std::vector<double> XC(3);
                System::arraycopy(X, 0, XC, 0, 3);
                std::vector<bool> fxc(3);
                System::arraycopy(fixed, 0, fxc, 0, 3);

                return new Point(label, XC, fxc);
            }

        void copyPositionFrom(Point *pt)
            {
                X[0] = pt->X[0];
                X[1] = pt->X[1];
                X[2] = pt->X[2];

                XX[0] = pt->XX[0];
                XX[1] = pt->XX[1];
                XX[2] = pt->XX[2];
            }

       		std::string toString()
            {
				return label + L" [" + (fixed[0] ? L"*" : L" ") + L" " + String::format(L"%3.3f", X[0]) + L", " + (fixed[1] ? L"*" : L" ") + L" " + String::format(L"%3.3f", X[1]) + L", " + (fixed[2] ? L"*" : L" ") + L" " + String::format(L"%3.3f", X[2]) + L"]";
            }
    }

    static void printDistanceMatrix(vector<vector<double>> &dm)
    {
        cout << "Distance matrix" << endl;
        for (int i = 0; i < labels->length; i++)
        {
            if (i == 0)
            {
                cout << "  ";
            }
            printf("    %2s   ", labels[i]);
        }
        cout << endl;

        for (int i = 0; i < labels->length; i++)
        {
            for (int j = 0; j < labels->length; j++)
            {
                if (j == 0)
                {
                    printf(" %2s ",labels[i]);
                }
                if (dm[i][j] >= 0)
                {
                    printf("  %06.2f", dm[i][j]);
                }
                else
                {
                    cout << "         ";
                }
            }
            cout << endl;
        }
    }


    int main(int argc, char *argv[])
    {
        // System.out.println(args[0]);
        // File src = new File(args[0]);
        File *src = new File("RawPointMeasurements.txt");

        if (src->isDirectory())
        {
            src = new File(src, L"RawPointMeasurements.txt");
        }

        set labelSet = set();
        unordered_map pointMap = map();
        unordered_map buffer = unordered_map();
        int cmConverstion = 1;
        vector tableHeaderLabels;

        FileReader tempVar_1(src);
		BufferedReader *br = new BufferedReader(&tempVar_1);

        for (int lineNumber = 1; true; lineNumber++)
	    {
			string line = br::readLine();
			if (line == "")
			{
				break;
			}

			line = StringHelper::trim(line);
			if (line.length() == 0)
			{
				continue;
			}
			if (StringHelper::startsWith(line, "#")) // a comment
			{
				continue;
			}

			if ((int)line.find("[") > 0) // its a constraint and/or initial guess on the 3D point coordinates
			{
				tableHeaderLabels = nullptr;

				line = StringHelper::replace(line, "[", "")->replace("]", "");
				line = StringHelper::replace(line, ",", " ");

				line = StringHelper::replace(line, "  ", " ");
				line = StringHelper::replace(line, "  ", " ");
				line = StringHelper::replace(line, "  ", " ");

				vector<string> parts = line.split(" ");

				vector<double> pos(3);
				vector<bool> fixed(3);

				wstring label = parts[0];
				for (int i = 0; i < 3; i++)
				{
					string spec = parts[i + 1];
					if (StringHelper::startsWith(spec, L"*"))
					{
						pos[i] = static_cast<Double>(StringHelper::trim(spec.substr(1)));
						fixed[i] = true;
					}
					else
					{
						pos[i] = static_cast<Double>(StringHelper::trim(spec));
						fixed[i] = false;
					}
				}

				Point tempVar(label, pos, fixed);
				pointMap::put(label, &tempVar);
				labelSet->add(label);
			}
			else if (line.find(L"\t") != string::npos)
			{
				vector<string> parts = line.split("\t");
				if (parts.size() < 2)
				{
					throw IllegalStateException(StringHelper::wstring_to_string(L"Error on line " + std::to_wstring(lineNumber) + L" expected tabular distances (requires at least 2 tab-separated values)"));
				}

				if (tableHeaderLabels == nullptr)
				{
					tableHeaderLabels =vector();
					if (StringHelper::trim(parts[0]).equals("$tm"))
					{
						cmConverstion = 100;
					}
					else if (StringHelper::trim(parts[0]).equals(L"$tcm"))
					{
						cmConverstion = 1;
					}
					else
					{
						throw IllegalStateException(StringHelper::wstring_to_string(L"Expected table signifier $tm or $tcm (for a table in metres or a table in centimetres respectively) at line " + std::to_wstring(lineNumber)));
					}

					for (int i = 1; i < parts.size(); i++)
					{
						std::wstring label = StringHelper::trim(parts[i]);
						if (!labelSet->contains(label))
						{
							throw IllegalStateException(StringHelper::wstring_to_string(L"Unknown label " + label + L" on line " + std::to_wstring(lineNumber)));
						}

						tableHeaderLabels->add(label);
					}
				}
				else
				{
					std::wstring originPoint = StringHelper::trim(parts[0]);
					if (!labelSet->contains(originPoint))
					{
						throw IllegalStateException(StringHelper::wstring_to_string(L"Unknown label " + originPoint + L" on line " + std::to_wstring(lineNumber)));
					}

					std::unordered_map m = static_cast<std::unordered_map>(buffer->get(originPoint));
					if (m.empty())
					{
						m = std::unordered_map();
						buffer::put(originPoint, m);
					}

					for (int i = 1; i < parts.size(); i++)
					{
						std::wstring dd = StringHelper::trim(parts[i]);
						if (dd == L"-")
						{
							continue;
						}
						double distance = std::stod(dd) * cmConverstion;
						m.emplace(tableHeaderLabels->get(i - 1), distance);
					}
				}
			}
			else
			{
				std::vector<std::wstring> parts = line.split(L" ");
				if (parts.size() != 3)
				{
					throw IllegalStateException(StringHelper::wstring_to_string(L"Error on line " + std::to_wstring(lineNumber) + L" expected simple triple: label label distance"));
				}
				tableHeaderLabels = nullptr;

				std::wstring originPoint = parts[0];
				std::wstring destinationPoint = parts[1];
				double distance = static_cast<Double>(StringHelper::trim(parts[2]));

				std::unordered_map m = static_cast<std::unordered_map>(buffer->get(originPoint));
				if (m.empty())
				{
					m = std::unordered_map();
					buffer::put(originPoint, m);
				}

				m.emplace(destinationPoint, distance);
				if (!labelSet->contains(originPoint))
				{
					throw IllegalStateException(StringHelper::wstring_to_string(L"Unknown label " + originPoint + L" on line " + std::to_wstring(lineNumber)));
				}
				if (!labelSet->contains(destinationPoint))
				{
					throw IllegalStateException(StringHelper::wstring_to_string(L"Unknown label " + destinationPoint + L" on line " + std::to_wstring(lineNumber)));
				}
			}
	   }

       	br->close();

		labels = static_cast<std::vector<std::wstring>>(labelSet::toArray(std::vector<std::wstring>(0)));
        //JAVA TO C++ CONVERTER NOTE: The following call to the 'RectangularVectors' helper class reproduces the rectangular array initialization that is automatic in Java:
        //ORIGINAL LINE: dm = new double[labels.length][labels.length];
		dm = RectangularVectors::RectangularDoubleVector(labels->length, labels->length);
		for (int i = 0; i < dm->length; i++)
		{
			Arrays::fill(dm[i], -1);
		}

		for (int i = 0; i < labels->length; i++)
		{
			for (int j = 0; j < labels->length; j++)
			{
				dm[i][i] = 0;

				std::unordered_map m = static_cast<std::unordered_map>(buffer->get(labels[i]));
				if (m.empty())
				{
					continue;
				}
				std::optional<double> dd = static_cast<std::optional<double>>(m[labels[j]]);
				if (dd)
				{
					dm[i][j] = dd.value();
					dm[j][i] = dd.value();
				}
			}
		}

		points = static_cast<std::vector<Point*>>(pointMap::values().toArray(std::vector<Point*>(0)));

		std::wcout << L"Raw points (with constraints)" << std::endl;
		for (int i = 0; i < points->length; i++)
		{
			std::wcout << points[i] << std::endl;
		}

		printDistanceMatrix(dm);

		//Now iterate over potential coordinates to minimise errors
		lowestError = 0;
		for (int i = 0; i < dm->length - 1; i++)
		{
			for (int j = i + 1; j < dm->length; j++)
			{
				double ideal = dm[i][j];
				if (ideal > 0)
				{
					lowestError += std::abs(points[i].distance(points[j]) - ideal);
				}
			}
		}

		int tcount = Runtime::getRuntime().availableProcessors();
		std::vector<Runnable> jobs;
		for (int t = 0; t < tcount; t++)
		{
			std::vector<Point*> pts(points->length);
			for (int i = 0; i < points->length; i++)
			{
				pts[i] = points[i].copyOf();
			}

			int index = t;
			jobs.push_back([&] ()
			{
					 double scale = 20;
					 double localMinError = lowestError;

					 for (long long itt = 0; true; itt++)
					 {
						 bool improved = false;

						 for (int itt2 = 0; itt2 < 1000; itt2++)
						 {
							 for (int pc = 0; pc < pts.size(); pc++)
							 {
								 pts[pc]->randomShift(scale);
							 }

							 totalCounter::incrementAndGet();
							 double error = 0;
							 for (int i = 0; i < dm->length - 1; i++)
							 {
								 for (int j = i + 1; j < dm->length; j++)
								 {
									 double ideal = dm[i][j];
									 if (ideal > 0)
									 {
										 error += std::abs(pts[i]->perturbedDistance(pts[j]) - ideal);
									 }
								 }
							 }

							 if (error < localMinError)
							 {
								 for (int pc = 0; pc < pts.size(); pc++)
								 {
									 pts[pc]->acceptShiftedValues();
								 }
								 improved = true;
								 localMinError = error;

								 {
									 std::scoped_lock<std::mutex> lock(points);
									 if (localMinError < lowestError)
									 {
										 for (int i = 0; i < points->length; i++)
										 {
											 points[i].copyPositionFrom(pts[i]);
										 }
										 lowestError = localMinError;
									 }
									 else
									 {
										 for (int i = 0; i < points->length; i++)
										 {
											 pts[i]->copyPositionFrom(points[i]);
										 }
										 localMinError = lowestError;
									 }
								 }
							 }
						 }

						 if (!improved)
						 {
							 scale *= 0.999;
						 }

						 if ((itt % 100) == 0)
						 {
							 std::wcout << L"Current Error " << index << L"   " << localMinError << L"  (" << lowestError << L") scale " << scale << std::endl;
							 if (scale < 0.01)
							 {
								 break;
							 }
						 }

						 {
							 std::scoped_lock<std::mutex> lock(points);
							 if (localMinError > 1.2 * lowestError)
							 {
								 for (int i = 0; i < points->length; i++)
								 {
									 pts[i]->copyPositionFrom(points[i]);
								 }
								 localMinError = lowestError;
							 }
						 }
					 }


					 ExecutionService::passThroughMemoryBarrier();
			});
		}

        ExecutionService::executeJobsInParallel(jobs);

		std::wcout << L"Final points (with constraints)" << std::endl;
		for (int i = 0; i < points->length; i++)
		{
			std::wcout << points[i] << std::endl;
		}

        //JAVA TO C++ CONVERTER NOTE: The following call to the 'RectangularVectors' helper class reproduces the rectangular array initialization that is automatic in Java:
        //ORIGINAL LINE: double[][] dm2 = new double[points.length][points.length];
		std::vector<std::vector<double>> dm2 = RectangularVectors::RectangularDoubleVector(points->length, points->length);
		for (int i = 0; i < points->length; i++)
		{
			for (int j = 0; j < points->length; j++)
			{
				if (dm[i][j] > 0)
				{
					dm2[i][j] = points[i].distance(points[j]);
				}
				else
				{
					dm2[i][j] = -1;
				}
			}
		}

		std::wcout << L"\n\nOriginal Distance Matrix" << std::endl;
		printDistanceMatrix(dm);

		std::wcout << L"\n\nComputed Distance Matrix" << std::endl;
		printDistanceMatrix(dm2);

		double maxAbsError = 0;
		for (int i = 0; i < points->length; i++)
		{
			for (int j = 0; j < points->length; j++)
			{
				if (dm[i][j] > 0)
				{
					maxAbsError = std::max(maxAbsError, std::abs(dm[i][j] - dm2[i][j]));
				}
			}
		}

		std::wcout << L"\n\nMax absolute error:  " << maxAbsError << std::endl;

		std::wcout << L"\n-----------------\n     Complete Filled Out Distances \n" << std::endl;
		for (int i = 0; i < points->length; i++)
		{
			for (int j = 0; j < points->length; j++)
			{
				dm2[i][j] = points[i].distance(points[j]);
			}
		}
		printDistanceMatrix(dm2);

		std::wcout << L"\n\nFinal Points" << std::endl;
		for (int i = 0; i < points->length; i++)
		{
			std::wcout << points[i] << std::endl;
		}

		std::wcout << L"Writing final points to solution file - alphabetic named points only. Tested " << totalCounter << L" points" << std::endl;

        //JAVA TO C++ CONVERTER TODO TASK: There is no C++ equivalent to 'toString':
		File *output = new File(src->toString() + L"soln.txt");
		FileOutputStream tempVar(output);
		PrintStream *ps = new PrintStream(&tempVar);
		for (int i = 0; i < points->length; i++)
		{
			if (!std::isdigit(points[i].label->charAt(0)))
			{
				ps->println(points[i]);
			}
		}
		ps->close();

		return 0;
	}
};

