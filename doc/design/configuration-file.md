The ESDM Configuration File
===========================

Search Paths
------------

TODO

File Format
-----------

The configuration file format is based on JSON, i.e. all configuration files must be valid JSON files.
However, ESDM places some restrictions on what keys are recognized and which types are expected where:

  * The root of the configuration file is a dictionary that contains the following keys:
  
      * "esdm" (required):
        A dictionary that contains the following keys:

          * "backends" (required):
            An array of dictionarlies containing the following keys:

              * "type" (required):
                TODO

              * "id" (required):
                TODO

              * "target" (required):
                TODO

              * "max-fragment-size" (optional):
                The amount of data that may be written into a single fragment.
                The amount is given in bytes.

              * "fragmentation-method" (optional):
                A string identifying the algorithm to use to split a chunk of data into fragments.
                Legal values are:

                  * "contiguous":
                    This algorithm tries to form fragments that are scattered across memory as little as possible.
                    As such, it is expected to yield the best possible write performance.
                    However, if a transposition is performed when reading the data back, performance may be poor.

                    Splitting a dataspace of size (50, 80, 100) into fragments of 2000 entries results in fragments of size (1, 20, 100).

                  * "equalized":
                    This algorithm tries to form fragments that have a similar size in all dimensions.
                    As such, it is expected to perform equally well with all kinds of read requests,
                    but it tends to write scattered data to disk which has to be sequentialized first, imposing a performance penalty on the write side.

                    Splitting a dataspace of size (50, 80, 100) into fragments of at most 2000 entries results in fragments of sizes between (10, 11, 11) and (10, 12, 12).

          * "metadata" (required):
            A dictionary containing the following keys:

              * "type" (required):
                TODO

              * "id" (required):
                TODO
