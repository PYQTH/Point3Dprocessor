// package util;

import java.io.*;
import java.util.*;
import java.util.function.*;
import java.util.concurrent.*;


public class ExecutionService
{
    public static final int CORES = Runtime.getRuntime().availableProcessors();
    private static ExecutorService exec = Executors.newFixedThreadPool(Runtime.getRuntime().availableProcessors(), (r)-> { Thread result = new Thread(r); result.setDaemon(true); return result;});
    private static long uselessCounter = 0;

    private static void waitForCurrentResults(Future[] ff)
    {
        while (true)
        {
            int completeCount = 0;

            for (int i=0; i<ff.length; i++)
                try
                {
                    ff[i].get(1000, TimeUnit.MILLISECONDS);
                    if (ff[i].isDone())
                        completeCount++;
                }
                catch (InterruptedException | TimeoutException e) {}
                catch (Exception e)
                {
                    //The future threw an exception, at least it completed!
                    completeCount++;
                    throw new IllegalStateException(e);
                }

            synchronized (exec)
            {
                uselessCounter += 1;
                if (completeCount >= ff.length)
                    return;
            }

            try
            {
                Thread.sleep(100);
            }
            catch (Exception e) {}
        }
    }

    public static void passThroughMemoryBarrier()
    {
        synchronized (exec)
        {
            uselessCounter += 1;
        }
    }

    public static void executeJobsInParallel(Runnable[] jobs)
    {
        Future[] ff = new Future[jobs.length];
        for (int i=0; i<jobs.length; i++)
            ff[i] = exec.submit(jobs[i]);
        waitForCurrentResults(ff);
    }

    public static void executeJobsInSeries(Runnable[] jobs)
    {
        for (int i=0; i<jobs.length; i++)
            jobs[i].run();
    }

    public static void executeJobsInParallel(List<Runnable> jobs)
    {
        Future[] ff = new Future[jobs.size()];
        for (int i=0; i<ff.length; i++)
            ff[i] = exec.submit(jobs.get(i));
        waitForCurrentResults(ff);
    }

    public static void executeJobsInSeries(List<Runnable> jobs)
    {
        for (int i=0; i<jobs.size(); i++)
            jobs.get(i).run();
    }
}
