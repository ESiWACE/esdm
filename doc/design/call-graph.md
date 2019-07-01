# Callgraph for accessing metadata

Guiding question:
  * How and when do we fetch metadata in the read path?
  * When do we serialize metadata to JSON in the write path?

## General responsibility

  * Container holds references to datasets (under different name possibly)
  * Dataset holds information about the dataset itself:
    * User-defined attributes (scientific metadata) => it is not know a-priori what that is
    * Technical attributes (some may be optional) => well known how they look like
    * Fragment information is directly inlined as part of datasets
  * Fragments
    * Information about their shape, backend plugin ID, backend-specific options (unknown to us)

## Write

Applies for datasets, containers, fragments:
  * All data is kept in appropriate structures in main memory
  * Serialize to JSON just before calling the metadata backend to store the metadata (and free JSON afterwards)
  * Backend communicates via JSON to ESDM layer

Callgraph from user perspective:
  * c = container_create("name")
  * d = dataset_create(c, "dset")
  * write(d) creates fragments and attaches the metadata to the dataset
  * dataset_commit(d) => make the dataset persistent, also write the dataset + fragment metadata
  * container_commit(c) => makes the container persistent
          TODO: check that the right version of data is linked to it.
  * dataset_destroy(d)
  * container_destroy(c)

## Read

Applies for datasets, containers, fragments:
  * All data is kept in appropriate structures in main memory AND fetched when the data is queried initially
  * De-serialized from JSON at the earliest convenience in the ESDM layer
  * Backend communicates via JSON to ESDM layer

Callgraph from user perspective:
  * c = container_open("name") <= here we read the metadata for "name" and generate appropriate structures
  * d = dataset_open("dset") <= here we read the metadata for "dset" and generate appropriate structures
  * read(d)
  * dataset_destroy(d)
  * container_destroy(c)

Alternative workflow with unkown dsets:
  * c = container_open("name") <= here we read the metadata for "name" and generate appropriate structures
  * it = dataset_iterator()
  * for x in it:
    * dataset_iterator_dataset(x)
  * read(d)
  * dataset_destroy(d)
  * container_destroy(c)
