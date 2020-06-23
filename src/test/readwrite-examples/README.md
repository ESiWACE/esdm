Example instructions for the read-write benchmark
=================================================

This directory contains instruction files for use with the readwrite-benchmark.
**Be warned, some of these write/read quite a lot of data!**

These are organized into a number of different scenarios.
Some scenarios consist only of a single file that is supposed to be used both for writing and reading.
More interesting scenarios consist of separate read/write instruction files,
these end either in the suffix "-write" or "-read" after a common prefix.
Some more complex scenarios may contain several "-write" or "-read" file,
in which case a sequence number is attached at the end of the file name.

The different instruction sets are described below:

  * climate-analysis-40

      * -write

        Simulates output of a single variable (8bytes per value) from a climate model using a 500x1000 lat/lon grid.
        Output is performed on a daily basis over the course of one year, with an ensemble of five runs.
        Since this simulates the output of an ensemble simulation run,
        each write request touches only a single time point / ensemble member.
        Total written data size is 8B * 500*1000 * 365 * 5 = 7.3GB

      * -read1, -read2, -read3

        Simulates read operation for different regional analysis.
        The analysed areas overlap strongly.
        Since this simulates a statistical analysis, each read requests data from all timesteps and ensemble members at once.
        The data is partitioned along the surface coordinates, only.

         1. Regional analysis of 125x250 ground cells over the entire year.
            Reads a total of 456MB of data.

         2. Regional, seasonal analysis of 150x200 ground cells over half a year.
            Reads a total of 216MB of data.

         3. Zonal, seasonal analysis of 100x1000 ground cells over three months.
            Reads a total of 360MB of data.
