#include <vector>
#include <stdexcept>
#include <mutex>
using namespace std;


class ExecutionService
{
    public:
        static const int CORES = Runtime::getRuntime().availableProcessors();
    private:
    //JAVA TO C++ CONVERTER TODO TASK: C++ does not allow initialization of static non-const/integral fields in their declarations - choose the conversion option for separate .h and .cpp files:
        static ExecutorService *exec = Executors::newFixedThreadPool(Runtime::getRuntime().availableProcessors(), [&] (r)
        {
            Thread *result = new Thread(r);
            result->setDaemon(true);

            delete result;
            
            return result;

    //JAVA TO C++ CONVERTER TODO TASK: A 'delete result' statement was not added since result was used in a 'return' or 'throw' statement.
        });
    //JAVA TO C++ CONVERTER TODO TASK: C++ does not allow initialization of static non-const/integral fields in their declarations - choose the conversion option for separate .h and .cpp files:
        static long long uselessCounter = 0;

        static void waitForCurrentResults(std::vector<Future*> &ff)
        {
            while (true)
            {
                int completeCount = 0;

                for (int i = 0; i < ff.size(); i++)
                {
                    try
                    {
                        ff[i]->get(1000, TimeUnit::MILLISECONDS);
                        if (ff[i]->isDone())
                        {
                            completeCount++;
                        }
                    }
                    //JAVA TO C++ CONVERTER TODO TASK: There is no equivalent in C++ to Java 'multi-catch' syntax:
                    catch (InterruptedException | TimeoutException e)
                    {
                    }
                    catch (const std::runtime_error &e)
                    {
                        //The future threw an exception, at least it completed!
                        completeCount++;
                        throw IllegalStateException(e);
                    }
                }

                {
                    std::scoped_lock<std::mutex> lock(exec);
                    uselessCounter += 1;
                    if (completeCount >= ff.size())
                    {
                        return;
                    }
                }

                try
                {
                    delay(100);
                }
                catch (const std::runtime_error &e)
                {
                }
            }
        }

    public:
        static void passThroughMemoryBarrier()
        {
            {
                std::scoped_lock<std::mutex> lock(exec);
                uselessCounter += 1;
            }
        }

        static void executeJobsInParallel(std::vector<Runnable> &jobs)
        {
            std::vector<Future*> ff(jobs.size());
            for (int i = 0; i < jobs.size(); i++)
            {
                ff[i] = exec->submit(jobs[i]);
            }
            waitForCurrentResults(ff);
        }

        static void executeJobsInSeries(std::vector<Runnable> &jobs)
        {
            for (int i = 0; i < jobs.size(); i++)
            {
                jobs[i]();
            }
        }

        static void executeJobsInParallel(std::vector<Runnable> &jobs)
        {
            std::vector<Future*> ff(jobs.size());
            for (int i = 0; i < ff.size(); i++)
            {
                ff[i] = exec->submit(jobs[i]);
            }
            waitForCurrentResults(ff);
        }

        static void executeJobsInSeries(std::vector<Runnable> &jobs)
        {
            for (int i = 0; i < jobs.size(); i++)
            {
                jobs[i]();
            }
	    }
};
