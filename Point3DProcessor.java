// package calibration;

import java.io.*;
import java.util.*;
import java.util.concurrent.atomic.*;

// import util.*;

public class Point3DProcessor
{
    private static String[] labels;
    private static Point[] points;
    private static double[][] dm;

    private static double lowestError;
    private static AtomicLong totalCounter = new AtomicLong();

    static class Point
    {
        String label;
        boolean[] fixed;
        double[] X, XX;
        Random random;

        Point(String label, double[] X, boolean[] fx)
        {
            this.label = label;
            this.X = X;
            this.fixed = fx;

            XX = new double[3];
            XX[0] = X[0];
            XX[1] = X[1];
            XX[2] = X[2];

            random = new Random();
        }

        void randomShift(double scale)
        {
            for (int i=0; i<3; i++)
            {
                XX[i] = X[i];
                if (fixed[i])
                    continue;

                XX[i] += random.nextGaussian()*scale;
            }
        }

        void acceptShiftedValues()
        {
            System.arraycopy(XX, 0, X, 0, 3);
        }

        double distance(Point pt)
        {
            double dx = X[0] - pt.X[0];
            double dy = X[1] - pt.X[1];
            double dz = X[2] - pt.X[2];

            return Math.sqrt(dx*dx + dy*dy + dz*dz);
        }

        double perturbedDistance(Point pt)
        {
            double dx = XX[0] - pt.XX[0];
            double dy = XX[1] - pt.XX[1];
            double dz = XX[2] - pt.XX[2];

            return Math.sqrt(dx*dx + dy*dy + dz*dz);
        }

        Point copyOf()
        {
            double[] XC = new double[3];
            System.arraycopy(X, 0, XC, 0, 3);
            boolean[] fxc = new boolean[3];
            System.arraycopy(fixed, 0, fxc, 0, 3);

            return new Point(label, XC, fxc);
        }

        void copyPositionFrom(Point pt)
        {
            X[0] = pt.X[0];
            X[1] = pt.X[1];
            X[2] = pt.X[2];

            XX[0] = pt.XX[0];
            XX[1] = pt.XX[1];
            XX[2] = pt.XX[2];
        }

        public String toString()
        {
            return label+" ["+(fixed[0] ? "*" : " ")+" "+String.format("%3.3f", X[0])+", "+(fixed[1] ? "*" : " ")+" "+String.format("%3.3f", X[1])+", "+(fixed[2] ? "*" : " ")+" "+String.format("%3.3f", X[2])+"]";
        }
    }

    private static void printDistanceMatrix(double[][] dm)
    {
        System.out.println("Distance matrix");
        for (int i=0; i<labels.length; i++)
        {
            if (i == 0)
                System.out.print("  ");
            System.out.printf("    %2s   ", labels[i]);
        }
        System.out.println();

        for (int i=0; i<labels.length; i++)
        {
            for (int j=0; j<labels.length; j++)
            {
                if (j == 0)
                    System.out.printf("%2s ",labels[i]);
                if (dm[i][j] >= 0)
                    System.out.printf("  %06.2f", dm[i][j]);
                else
                    System.out.print("         ");
            }
            System.out.println();
        }
    }

    public static void main(String[] args) throws Exception
    {
        // System.out.println(args[0]);
        // File src = new File(args[0]);
        File src = new File("RawPointMeasurements.txt");

        if (src.isDirectory())
            src = new File(src, "RawPointMeasurements.txt");

        TreeSet labelSet = new TreeSet();
        Map pointMap = new TreeMap();
        HashMap buffer = new HashMap();
        int cmConverstion = 1;
        ArrayList tableHeaderLabels = null;

        BufferedReader br = new BufferedReader(new FileReader(src));
        // System.out.println("This is buffer:");
        // System.out.println(br);
        for (int lineNumber=1; true; lineNumber++)
        {
            String line = br.readLine();
            // System.out.println(line);
            if (line == null)
                break;

            line = line.trim();
            if (line.length() == 0)
                continue;
            if (line.startsWith("#")) // a comment
                continue;
            // System.out.println(line);
            // System.out.println(line.indexOf("["));
            // System.out.println(line.indexOf("/t"));
            
            if (line.indexOf("[") > 0) // its a constraint and/or initial guess on the 3D point coordinates
            {
                
                tableHeaderLabels = null;

                line = line.replace("[", "").replace("]", "");
                line = line.replace(",", " ");

                line = line.replace("  ", " ");
                line = line.replace("  ", " ");
                line = line.replace("  ", " ");

                String[] parts = line.split(" ");

                double[] pos = new double[3];
                boolean[] fixed = new boolean[3];

                String label = parts[0];
                for (int i=0; i<3; i++)
                {
                    String spec = parts[i+1];
                    if (spec.startsWith("*"))
                    {
                        pos[i] = Double.parseDouble(spec.substring(1).trim());
                        fixed[i] = true;
                    }
                    else
                    {
                        pos[i] = Double.parseDouble(spec.trim());
                        fixed[i] = false;
                    }
                }

                pointMap.put(label, new Point(label, pos, fixed));
                labelSet.add(label);
            }
            else if (line.indexOf("\t") >= 0)
            {
                String[] parts = line.split("\t");
                // System.out.println(parts);
                if (parts.length < 2)
                    throw new IllegalStateException("Error on line "+lineNumber+" expected tabular distances (requires at least 2 tab-separated values)");

                if (tableHeaderLabels == null)
                {
                    tableHeaderLabels = new ArrayList();
                    if (parts[0].trim().equals("$tm"))
                        cmConverstion = 100;
                    else  if (parts[0].trim().equals("$tcm"))
                        cmConverstion = 1;
                    else
                        throw new IllegalStateException("Expected table signifier $tm or $tcm (for a table in metres or a table in centimetres respectively) at line "+lineNumber);

                    for (int i=1; i<parts.length; i++)
                    {
                        String label = parts[i].trim();
                        if (!labelSet.contains(label))
                            throw new IllegalStateException("Unknown label "+label+" on line "+lineNumber);

                        tableHeaderLabels.add(label);
                    }
                }
                else
                {
                    String originPoint = parts[0].trim();
                    if (!labelSet.contains(originPoint))
                        throw new IllegalStateException("Unknown label "+originPoint+" on line "+lineNumber);

                    Map m = (Map) buffer.get(originPoint);
                    if (m == null)
                    {
                        m = new HashMap();
                        buffer.put(originPoint, m);
                    }

                    for (int i=1; i<parts.length; i++)
                    {
                        String dd = parts[i].trim();
                        if (dd.equals("-"))
                            continue;
                        double distance = Double.parseDouble(dd) * cmConverstion;
                        m.put(tableHeaderLabels.get(i-1), distance);
                    }
                }
            }
            else
            {
                String[] parts = line.split(" ");
                // System.out.println(parts.length);
                // System.out.println(parts[0]);
                // System.out.println(parts[1]);
                // System.out.println(parts[2]);

                if (parts.length != 3)
                    throw new IllegalStateException("Error on line "+lineNumber+" expected simple triple: label label distance");
                tableHeaderLabels = null;

                String originPoint = parts[0];
                String destinationPoint = parts[1];
                double distance = Double.parseDouble(parts[2].trim());

                Map m = (Map) buffer.get(originPoint);
                // System.out.println(m);
                
                if (m == null)
                {
                    m = new HashMap();
                    buffer.put(originPoint, m);
                }

                m.put(destinationPoint, distance);
                if (!labelSet.contains(originPoint))
                    throw new IllegalStateException("Unknown label "+originPoint+" on line "+lineNumber);
                if (!labelSet.contains(destinationPoint))
                    throw new IllegalStateException("Unknown label "+destinationPoint+" on line "+lineNumber);
            }
        }
        // System.out.println(m);
        br.close();

        
        labels = (String[]) labelSet.toArray(new String[0]);
        // System.out.println(labels());
        dm = new double[labels.length][labels.length];
        for (int i=0; i<dm.length; i++){
            Arrays.fill(dm[i], -1);
            // System.out.println(dm[i][i]); 
        }

        for (int i=0; i<labels.length; i++)
            for(int j=0; j<labels.length; j++)
            {
                dm[i][i] = 0;

                Map m = (Map) buffer.get(labels[i]);

                System.out.println(m);

                if (m == null)
                    continue;
                Double dd = (Double) m.get(labels[j]);
                if (dd != null)
                {
                    dm[i][j] = dd.doubleValue();
                    dm[j][i] = dd.doubleValue();
                }
            }

        points = (Point[]) pointMap.values().toArray(new Point[0]);

        System.out.println("Raw points (with constraints)");
        for (int i=0; i<points.length; i++)
            System.out.println(points[i]);

        printDistanceMatrix(dm);

        //Now iterate over potential coordinates to minimise errors
        lowestError = 0;
        for (int i=0; i<dm.length-1; i++)
            for (int j=i+1; j<dm.length; j++)
            {
                double ideal = dm[i][j];
                if (ideal > 0)
                    lowestError += Math.abs(points[i].distance(points[j]) - ideal);
            }

        int tcount = Runtime.getRuntime().availableProcessors();
        List<Runnable> jobs = new ArrayList<>();
        for (int t=0; t<tcount; t++)
        {
            Point[] pts = new Point[points.length];
            for (int i=0; i<points.length; i++)
                pts[i] = points[i].copyOf();

            int index = t;
            jobs.add(()->
                 {
                     double scale = 20;
                     double localMinError = lowestError;

                     for (long itt=0; true; itt++)
                     {
                         boolean improved = false;

                         for (int itt2=0; itt2<1000; itt2++)
                         {
                             for (int pc=0; pc<pts.length; pc++)
                                 pts[pc].randomShift(scale);

                             totalCounter.incrementAndGet();
                             double error = 0;
                             for (int i=0; i<dm.length-1; i++)
                                 for (int j=i+1; j<dm.length; j++)
                                 {
                                     double ideal = dm[i][j];
                                     if (ideal > 0)
                                         error += Math.abs(pts[i].perturbedDistance(pts[j]) - ideal);
                                 }

                             if (error < localMinError)
                             {
                                 for (int pc=0; pc<pts.length; pc++)
                                     pts[pc].acceptShiftedValues();
                                 improved = true;
                                 localMinError = error;

                                 synchronized (points)
                                 {
                                     if (localMinError < lowestError)
                                     {
                                         for (int i=0; i<points.length; i++)
                                             points[i].copyPositionFrom(pts[i]);
                                         lowestError = localMinError;
                                     }
                                     else
                                     {
                                         for (int i=0; i<points.length; i++)
                                             pts[i].copyPositionFrom(points[i]);
                                         localMinError = lowestError;
                                     }
                                 }
                             }
                         }

                         if (!improved)
                             scale *= 0.999;

                         if ((itt % 100) == 0)
                         {
                             System.out.println("Current Error "+index+"   "+localMinError+"  ("+lowestError+") scale "+scale);
                             if (scale < 0.01)
                                 break;
                         }

                         synchronized (points)
                         {
                             if (localMinError > 1.2*lowestError)
                             {
                                 for (int i=0; i<points.length; i++)
                                     pts[i].copyPositionFrom(points[i]);
                                 localMinError = lowestError;
                             }
                         }
                     }

                     ExecutionService.passThroughMemoryBarrier();
                 });
        }

        ExecutionService.executeJobsInParallel(jobs);

        System.out.println("Final points (with constraints)");
        for (int i=0; i<points.length; i++)
            System.out.println(points[i]);

        double[][] dm2 = new double[points.length][points.length];
        for (int i=0; i<points.length; i++)
            for (int j=0; j<points.length; j++)
            {
                if (dm[i][j] > 0)
                    dm2[i][j] = points[i].distance(points[j]);
                else
                    dm2[i][j] = -1;
            }

        System.out.println("\n\nOriginal Distance Matrix");
        printDistanceMatrix(dm);

        System.out.println("\n\nComputed Distance Matrix");
        printDistanceMatrix(dm2);

        double maxAbsError = 0;
        for (int i=0; i<points.length; i++)
            for (int j=0; j<points.length; j++)
                if (dm[i][j] > 0)
                    maxAbsError = Math.max(maxAbsError, Math.abs(dm[i][j] - dm2[i][j]));

        System.out.println("\n\nMax absolute error:  "+maxAbsError);

        System.out.println("\n-----------------\n     Complete Filled Out Distances \n");
        for (int i=0; i<points.length; i++)
            for (int j=0; j<points.length; j++)
                dm2[i][j] = points[i].distance(points[j]);
        printDistanceMatrix(dm2);

        System.out.println("\n\nFinal Points");
        for (int i=0; i<points.length; i++)
            System.out.println(points[i]);

        System.out.println("Writing final points to solution file - alphabetic named points only. Tested "+totalCounter+" points");

        File output = new File(src.toString()+"soln.txt");
        PrintStream ps = new PrintStream(new FileOutputStream(output));
        for (int i=0; i<points.length; i++)
            if (!Character.isDigit(points[i].label.charAt(0)))
                ps.println(points[i]);
        ps.close();
    }
}
