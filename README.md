-   [Earth System Data Middleware](#earth-system-data-middleware)
    -   [Requirements](#requirements)
    -   [Installation](#installation)
        -   [Ubuntu 20.04](#ubuntu-2004)
    -   [Development](#development)
        -   [Project directory structure](#project-directory-structure)
-   [Building ESDM documentation](#building-esdm-documentation)
    -   [PDF](#pdf)
    -   [Doxygen](#doxygen)
    -   [Github-Markdown](#github-markdown)
-   [Data Model](#data-model)
    -   [Conceptual Data Model](#conceptual-data-model)
    -   [Logical Data Model](#logical-data-model)
    -   [The ESDM Configuration File](#the-esdm-configuration-file)
        -   [Search Paths](#search-paths)
        -   [File Format](#file-format)
        -   [Metadata](#metadata-2)
-   [Installation Instructions for
    Mistral](#installation-instructions-for-mistral)
    -   [Satisfying Requirements](#satisfying-requirements)
    -   [Download and enable Spack](#download-and-enable-spack)
    -   [Set a gcc version to be used](#set-a-gcc-version-to-be-used)
    -   [Install Dependencies](#install-dependencies)
    -   [Load Dependencies](#load-dependencies)
        -   [HDF5 with Virtual Object Layer
            Support](#hdf5-with-virtual-object-layer-support)
    -   [Ensure environment is aware of dependencies installed using
        spack and
        dev-env](#ensure-environment-is-aware-of-dependencies-installed-using-spack-and-dev-env)
    -   [Download HDF5 with VOL
        support](#download-hdf5-with-vol-support)
    -   [Configure and build HDF5](#configure-and-build-hdf5)
        -   [NetCDF with Support for ESDM VOL
            Plugin](#netcdf-with-support-for-esdm-vol-plugin)
    -   [Ensure environment is aware of dependencies installed using
        spack and
        dev-env](#ensure-environment-is-aware-of-dependencies-installed-using-spack-and-dev-env-1)
    -   [Downlaod NetCDF source](#downlaod-netcdf-source)
    -   [Apply patches to allow enabling/disabling
        plugin](#apply-patches-to-allow-enablingdisabling-plugin)
    -   [Configure and build](#configure-and-build)
        -   [Building ESDM Prototype](#building-esdm-prototype)
    -   [Ensure environment is aware of dependencies installed using
        spack and
        dev-env](#ensure-environment-is-aware-of-dependencies-installed-using-spack-and-dev-env-2)
    -   [Configure and build ESDM](#configure-and-build-esdm)
-   [Use Cases](#use-cases)
    -   [ Workloads in climate and
        weather](#workloads-in-climate-and-weather)
        -   [NWP](#nwp)
        -   [Climate](#climate)
    -   [Stakeholders and Systems](#stakeholders-and-systems)
    -   [List of Use-Cases](#list-of-use-cases)
-   [Styleguide for ESDM development](#styleguide-for-esdm-development)
    -   [General rules](#general-rules)
    -   [Naming conventions](#naming-conventions)
    -   [Example Code](#example-code)
-   [Developers Guide](#developers-guide)
    -   [Internal Data Model](#internal-data-model)
        -   [Dataspace](#dataspace)
        -   [Dataset](#dataset-1)
        -   [Container](#container-1)
        -   [Grid](#grid)
    -   [Usage Examples](#usage-examples)
        -   [Basic Writing](#basic-writing)
        -   [Grid Based Writing](#grid-based-writing)
        -   [Simple Reading](#simple-reading)
        -   [Grid Based Reading](#grid-based-reading)
-   [Callgraph for accessing
    metadata](#callgraph-for-accessing-metadata)
    -   [General responsibility](#general-responsibility)
    -   [Write](#write)
    -   [Read](#read)
-   [Grid deduplication](#grid-deduplication)
    -   [Requirements](#requirements-1)
    -   [Implementation](#implementation)
    -   [Roadmap for Implementation](#roadmap-for-implementation)
-   [Related Work](#related-work)

# Earth System Data Middleware

The middleware for earth system data is a prototype to improve I/O
performance for earth system simulation as used in climate and weather
applications. ESDM exploits structural information exposed by workflows,
applications as well as data description formats such as HDF5 and NetCDF
to more efficiently organize metadata and data across a variety of
storage backends.

## Requirements

A compiler for C99 such as GCC5.0

## Installation

Ensure you cloned the repository with the required submodules:

-   git clone --recurse-submodules Or to initialize the submodules after
    the cloning:

-   git submodule update --init --recursive

The installation instructions for the full stack with ESDM and NetCDF
can be found in the Dockerfiles that we use for testing.

### Ubuntu 20.04

$ grep RUN dev/docker/ubuntu-whole-stack/Dockerfile

#### Installation for CentOS7

$ grep RUN dev/docker/centos7-whole-stack/Dockerfile

#### Installation for FedoraSystem

-   dnf install glib2 glib2-devel mpi jansson jansson-devel

-   dnf install mpich-3.0 mpich-3.0-devel OR dnf install openmpi
    opemmpi-devel

-   dnf install gcc-c++ gcc libtool cmake

#### Installation with Spack

Installation of spack itself:

-   git clone <https://github.com/spack/spack>

-   export PATH=$PATH:$PWD/spack/bin/ Check that a suitable compiler is
    found

-   spack compilers

First get the recent GCC:

-   spack install [`gcc@9.3.0`](mailto:gcc@9.3.0) Then install the
    packages with GCC:

-   spack install jansson%[`gcc@9.3.0`](mailto:gcc@9.3.0)
    glib%[`gcc@9.3.0`](mailto:gcc@9.3.0)
    openmpi%[`gcc@9.3.0`](mailto:gcc@9.3.0)
    gettext%[`gcc@9.3.0`](mailto:gcc@9.3.0) Before running configure
    load the modules:

-   spack load jansson%[`gcc@9.3.0`](mailto:gcc@9.3.0)
    glib%[`gcc@9.3.0`](mailto:gcc@9.3.0)
    openmpi%[`gcc@9.3.0`](mailto:gcc@9.3.0) gcc
    gettext%[`gcc@9.3.0`](mailto:gcc@9.3.0) -r

## Development

### Project directory structure

-   `dev` contains helpers for development purposes. For example, this
    project requires a development variant of HDF5 that provides the
    Virtual Object Layer (VOL). This and other dependencies can be
    installed into a development environment using the following script:

            ./dev/setup-development-environment.sh

-   `src` contains the source code... To build the project call:

            source dev/activate-development-environment.bash
            ./configure --debug
            cd build
            make -j

    To run the test suite call:

            cd build
            make test

    You may also choose to configure with a different hdf5 installation
    (see ./configure --help) e.g.:

            ./configure --with-hdf5=$PWD/install

-   `tools` contains separate programs, e.g., for benchmarking HDF5.
    They should only be loosely coupled with the source code and allow
    to be used with the regular HDF5.

# Building ESDM documentation

<figure>
<img src="./doc/latex/../assets/build-docs-overview/graph.png" id="fig:build-docs-overview" style="width:70.0%" alt="Figure 1: Conversion paths." /><figcaption aria-hidden="true">Figure 1: Conversion paths.</figcaption>
</figure>

The documentation is organized as a set of latex and resource files.
This structure simplifies integration in other documents, if only
particular parts of the documentation are required. The source files are
located in `./doc/latex`. This is a central location, where files can be
modified, if changes are required. All other locations listed in this
sections are auto-generated. Therefore, after recompilation all changes
will be overwritten.

Documentation centralization reduces documentation efforts. As shown in
the latex source files are exported to different formats. Change in the
latex files will affect all documents.

PDF and Github-Markdown format don’t include API reference. You find API
reference in the Latex and HTML formats generated by `doxygen`.

The listings in this section assume that current working directory is
the ESDM repository. The subsections discuss the supported export
possibilities in detail.

## PDF

Latex documentation can be compiled by commands shown in . The result
will be stored in `./doc/latex/main.pdf`

<div id="lst:doc:latex" class="listing">

Listing lst:doc:latex: Make PDF document

```
cd ./doc
make
```

</div>

## Doxygen

Doxygen depends a set of auto-generated markdown files, which should
never be modified manually, because all changes will be overwritten by
next compilation. Please always work with files in `./doc/latex` folder
and compile them running the make script as shown in .

<div id="lst:dox:doxy-markdown" class="listing">

Listing lst:dox:doxy-markdown: Markdown files generation

```
cd ./doc
make
```

</div>

The resulting `*.md` files are generated in `./doc/markdown` directory.
Now all required source files for doxygen should be available and the
final documentation can compiled by `doxygen` as shown in .

<div id="lst:doc:doxygen" class="listing">

Listing lst:doc:doxygen: Make HTML and Latex documents

```
./configure
cd build
doxygen
```

</div>

The resulting HTML start page is located in
`./build/doc/html/index.html` and main Latex document is located in
`./build/doc/latex/refman.tex`.

## Github-Markdown

The Github documentation `./README.md` is generated by `pandoc` from the
`./doc/latex/main.tex` file. `./README.md` shold be never modified
manually, since changes will be overwritten by the next documentation
compilation. The compilation commands are shown in .

<div id="lst:doc:gfm" class="listing">

Listing lst:doc:gfm: Make ./README.md

```
cd ./doc
make
```

</div>

# Data Model

While datatypes introduced by computer architectures and software
libraries are important for the data model, they are discussed
separately in .

The data model of a system organizes elements of data, standardizes how
they represent data entities and how users can interact with the data.
The **model** can be split into three layers:

-   The **conceptual data** model describes the entities and the
    semantics of the domain that are represented by the data model and
    the typical operations to manipulate the data. In our case, the
    scientific domain is NWP/climate.

-   The **logical data model** describes the abstraction level provided
    by the system, how domain entities are mapped to objects provided by
    the system[1], and the supported operations to access and manipulate
    these objects are defined. Importantly, the **logical data model**
    defines the semantics when using the operations to access and
    manipulate the system objects. For example, a system object of a
    relational model is a table – representing attributes of a set of
    objects – and a row of a table representing attributes of a single
    object.

-   The physical data model describes how the logical entities are
    finally mapped to objects/files/regions on available hardware. The
    physical data model is partly covered by the backends of ESDM,
    therefore, the descriptions will stop at that abstraction level.

## Conceptual Data Model

Our conceptual data model is aligned with the needs of domain scientists
from climate and weather. It reuses and extends from concepts introduced
in a data model proposed for the Climate and Forecasting conventions for
NetCDF data.

#### Variable:

A variable, *V*, defines a set of data representing a discrete
(generally scalar) quantity discretised within a “sampling” domain, *d*.
It is accompanied by

#### Metadata:

Which will include at the minimum, a name, but may also include units,
and information about additional dimensionality, directly (e.g. via a
key, value pair such as that necessary to expose *z* = 1.5*m* for air
temperature at 1.5m) or indirectly (e.g. via pointers to other generic
coordinate variables which describe the sampled domain). There may also
be a dictionary of additional metadata which may or may not conform to
an external semantic convention or standard. Such metadata could include
information about the tool used to observe or simulate the specific
variable. Additional metadata is also required for all the other
entities described below.

#### Dimension:

The sampling domain *d* is defined by Dimensions which define an a
coordinate axis. Dimensions will also include metadata, which must
again, include at a minimum a name (e.g. height, time), but may also
include information about directionality, units (e.g. degrees, months,
days-since-a-particular-time-using-a-particular-calendar), or details of
how to construct an algorithm to find the actual sampling coordinates,
perhaps using a well known algorithm such as an ISO 8601 time.

#### Coordinate:

Coordinates are the set of values at which data is sampled along any
given dimension. They may be explicitly defined by indexing into a
coordinate variable, or implicitly defined by an algorithm. When we talk
about the coordinates, it is usually clear if we mean the N-dimensional
coordinate to address data in a given variable or if we just mean the
(1D) coordinate along one dimension.

#### Cell:

The data values are known at points, which may or may not represent a
cell. Such cells are n-dimensional shapes where the dimensionality may
or may not fully encompass the dimensionality of the domain.
n-dimensional shapes can be implicitly defined in which case the
Cartesian product of all dimensional coordinates forms the data "cube"
of the cell, but they can also be explicitly defined, either by
providing bounds on the coordinate variables (via metadata) or by
introducing a new variable which explicitly defines the functional
boundaries of the cell (as might happen in a finite element unstructured
grid).

#### Dataset:

Variables can be aggregated into datasets. A dataset contains multiple
variables that logically belong together, and should be associated with
metadata describing the reason for the aggregation. Variables must have
unique names within a dataset.

Our conceptual model assumes that all variables are scalars, but clearly
to make use of these scalars requires more complex interpretation. In
particular, we need to know the

#### Datatype:

which defines the types of values that are valid and the operations that
can be conducted. While we are mostly dealing with scalars, they may not
be amenable to interpretation as simple numbers. For example, a variable
may be storing an integer which points into a taxonomy of categories of
land-surface-types. More complex structures could include complex data
types such as vectors, compressed ensemble values, or structures within
this system, provided such interpretation is handled outside of the
ESDM, and documented in metadata. This allows us to limit ourselves to
simple data types plus arbitrary length blocks of bits.

#### Operators:

Define the manipulations possible on the conceptual entities. The
simplest operators will include creation, read, update and delete
applied to an entity as a whole, or to a subset, however even these
simple operators will come with constraints, for example, it should not
be possible to delete a coordinate variable without deleting the parent
variable as well. There will need to be a separation of concerns between
operators which can be handled *within* the ESDM subsystem, and those
which require external logic. Operators which might require external
logic could include subsetting — it will be seen that the ESDM will
support simple subsetting using simple coordinates — but complex subsets
such as finding a region in real space from dimensions spanned using an
algorithm or coordinate variable, may require knowledge of how such
algorithms or variables are specified. Such knowledge is embedded in
conventions such as the CF NetCDF conventions, and this knowledge could
only be provided to the ESDM via appropriate operator plugins.

## Logical Data Model

The logical data model describes how data is represented inside ESDM,
the operations to interact with the data and their semantics. There are
four first class entities in the ESDM logical data model: **variable**s,
**fragment**s, **container**s, and **metadata**. ESDM entities may be
linked by ESDM **reference**s, and a key property which emerges from the
use of references is that no ESDM entity instance may be deleted while
references to it still exist. The use of reference counting will ensure
this property as well as avoid dangling pointers.

gives an overview of the logical data model.

Each of these entities is now described, along with a list of supported
operations:

#### Variable:

In the logical data model, the variable corresponds directly to a
variable in the conceptual data model. Each element of the variable
sampled across the dimensions contains data with a prescribed
**DataType**. Variables are associated with both **Scientific Metadata**
and **Technical Metadata**. Variables are partitioned into **fragments**
each of which can be stored on one or more “storage backend.” A variable
definition includes internal information about the domain (bounding box
in some coordinate system) and dimensionality (size and shape), while
the detailed information about which coordinate variables are needed to
span the dimensions and how they are defined is held in the technical
metadata. Similarly, where a variable is itself a coordinate variable, a
link to the parent variable for which it is used is held in the
technical metadata. The ESDM will not allow an attempt to delete a
variable to succeed while any such references exist (see references). A
key part of the variable definition is the list of fragments associated
with it, and if possible, how they are organised to span the domain.
Users may choose to submit code pieces that are then run within the
I/O-path (not part within ESiWACE implementation), such an operation
covers the typical filter, map and reduce operations of the data flow
programming paradigm.

Fragments are created by the backend while appending/modifying data to a
variable.

Operations:

-   Variables can be created and deleted.

-   Fragments of data can be attached and deleted.

-   Fragments can be repartitioned and reshuffled.

-   Integrity can be checked.

-   Data can be read, appended or modified those operations will be
    translated to the responsible fragments.

-   Metadata can be atomically attached to a variable or modified.

-   A variable can be sealed to make it immutable for all subsequent
    modifications.

-   Process data of the variable somewhere in the I/O-path.

#### Fragment:

A fragment is a piece (subdomain) of a variable. The ESDM expects to
handle fragments as atomic entities, that is, only one process can write
a fragment through the ESDM, and the ESDM will write fragments as atomic
entities to storage backends. The backends are free to further partition
these fragments in an appropriate way, for example, by sharding using
chunks as described in the mapping section. However, the ESDM is free to
replicate fragments or subsets of fragments and to choose which backend
is appropriate for any given fragment. This allows, for example, the
ESDM to split a variable into fragments some of which are on stored on a
parallel file system, while others are placed in object storage.

Operations:

-   Data of fragments can be read, appended or modified.

-   Integrity of the fragment can be checked.

-   Process data of the variable somewhere in the I/O-path.

#### Container:

A container is a virtual entity providing views on collections of
variables, allowing multiple different datasets (as defined in the
conceptual model) to be realised over the variables visible to the ESDM.
Each container provides a hierarchical namespace holding references to
one or multiple variables together with metadata. Variables cannot be
deleted while they are referenced by a container. The use of these
dynamic containers provides support for much more flexible organisation
of data than provided by a regular file system semantics — and
efficiently support high level applications such as the Data Reference
Syntax[2].

A container provides the ESDM storage abstraction which is analogous to
an external file. Because many scientific metadata conventions are based
on semantic structures which span variables within a file in ways that
may be opaque to the ESDM without the use of a plugin, the use of a
container can indicate to the ESDM that these variables are linked even
though the ESDM does not understand why, and so they cannot be
independently deleted. When entire files in NetCDF format are ingested
into the ESDM, the respective importing tool must create a container to
ensure such linking properties are not lost.

Operations:

-   Creation and deletion of containers.

-   Creation and deletion of names in the hierarchical name space; the
    creation of links to an existing variable.

-   Attaching and modification of metadata.

-   Integrity can be checked.

#### Metadata:

Can be associated with all the other first class entities (variables,
fragments, and containers). Such metadata is split into internal ESDM
technical metadata, and external user-supplied semantic metadata.

Technical metadata covers, e.g., permissions, information about data
location and timestamps. A backend will be able to add its own metadata
to provide the information to lookup the data for the fragment from the
storage backend managed by it. Metadata by itself is treaded like a
normal ESDM variable but linked to the variable of choice. The
implementation may embed (simple) metadata into fragments of original
data (see Reference).

Operations:

-   Uses can create, read, or delete arbitrary scientific metadata onto
    variables and containers. A future version of the ESDM may support
    user scientific metadata for fragments.

-   Container level metadata is generally not directly associated with
    variables, but may be retrieved via following references from
    variables to containers.

-   Queries allow to search for arbitrary metadata, e.g., for objects
    that have (`experiment=X, model=Y, time=yesterday`) returning the
    variables and containers in a list that match. This enables to
    locate scientific data in an arbitrary namespace.

#### Reference:

A reference is a link between entities and can be used in many places,
references can be embedded instead of real data of these logical
objects. For example, dimensions inside a variable can be references,
also a container typically uses references to variables.

Operations:

-   A reference can be created from existing logical entities or
    removed.

#### Namespace:

ESDM does not offer a simple hierarchical namespace for the files. It
provides the elementary functions to navigate data: teleportation and
orientation in the following fashion: Queries about semantical data
properties (e.g.,
`experiment=myExperiment, model=myModel, date=yesterday`) can be
performed returning a list of matching files with their respective
metadata. Iterating the list (orientation) is similar to listing a
directory in a file system.

Note that this reduces the burden to define a hierarchical namespace and
for data sharing services based on scientific metadata. An input/output
container for an application can be assembled on the fly by using
queries and name the resulting entities. As a container provides a
hierachical namespace, by harnessing this capability one can search for
relevant variables and map them into the local file system tree,
accessing these variables as if they would be, e.g., NetCDF files. By
offering a FUSE client, this feature also enables backwards
compatibility for legacy POSIX applications.

## The ESDM Configuration File

### Search Paths

TODO

### File Format

The configuration file format is based on JSON, i.e. all configuration
files must be valid JSON files. However, ESDM places some restrictions
on what keys are recognized and which types are expected. The tables
summarize parameters that can be used in the configuration file.
Parameter can be of four types: string, integer, float and object, which
are collections of key-value pairs.

The “‘"esdm":“‘ key-value pair in the root of the JSON configuration
file can contain configuration for multiple data backends and one
metadata backend. The backends are organized in the “‘backends“‘
key-value pair as a list.

    {
      "esdm":	{
        "backends": [backend_1, backend_2, \ldots, backend_n],
        "metadata": {}
      }
    }

<div id="tab:backend_conf_params">

| Parameter              | Type    | Default    |          | Description                                   |
|:-----------------------|:--------|:-----------|:---------|:----------------------------------------------|
| type                   | string  | (not set)  | required | Backend name (see )                           |
| id                     | string  | (not set)  | required | Unique identifier string                      |
| target                 | string  | (not set)  | required | Connection specification (Bucket name for S3) |
| performance-model      | object  | (not set)  | optional | Performance model definition. (see )          |
| max-threads-per-node   | integer | 0          | optional | Maximum number of threads on a node.          |
| write-stream-blocksize | integer | 0          | optional | TODO.                                         |
| max-global-threads     | integer | 0          | optional | Maximum total number of threads.              |
| accessibility          | string  | global     | optional | TODO. (see )                                  |
| max-fragment-size      | integer | 10485760   | optional | Maximum fragment size in bytes.               |
| fragmentation-method   | string  | contiguous | optional | Fragmentation methods. (see )                 |
| host                   | string  | (not set)  | required | (S3 only) TODO                                |
| secret-key             | string  | (not set)  | required | (S3 only) TODO                                |
| access-key             | string  | (not set)  | required | (S3 only) TODO                                |
| locationConstraint     | string  | (not set)  | required | (S3 only) TODO                                |
| authRegion             | string  | (not set)  | required | (S3 only) TODO                                |
| timeout                | integer | (not set)  | required | (S3 only) TODO                                |
| s3-compatible          | integer | (not set)  | required | (S3 only) TODO                                |
| use-ssl                | integer | 0          | required | (S3 only) TODO                                |

Backend configuration parameters

</div>

#### Parameter: id

Unique identifier string.

#### Parameter: type and target

Can take a value of one of the supported backends listed in
@tbl:supportedtypes.

<div id="tab:supported_backends">

| Type   | Description                     |
|:-------|:--------------------------------|
| CLOVIS | Seagate Object Storage API      |
| DUMMY  | (Used for development)          |
| IME    | DDN Infinite Memory Engine      |
| KDSA   | Kove Direct System Architecture |
| POSIX  | POSIX interface                 |
| S3     | Amazon Simple Storage Service   |
| WOS    | DDN Object Storage              |

Supported backends

</div>

##### type = MOTR

    [local_addr] [ha_addr] [prof] [proc_fid]

where

-   "local\_addr" is the local address used to connect to Motr service,

-   "ha\_addr" is the hardware address in Motr service,

-   "prof" is the profile FID in the Motr service, and

-   "proc\_fid" is the process FID in the Motr service.

<!-- -->

      {
       "type": "MOTR",
       "id": "c1",
       "target": ":12345:33:103 192.168.168.146@tcp:12345:34:1 
         <0x7000000000000001:0> <0x7200000000000001:64>"
      }

##### type = DUMMY

(Used for development)

##### type = IME

TODO

      {
       "type": "IME",
       "id": "p1",
       "target": "./_ime"
      }

##### type = KDSA

Prefix “xpd:” followed by volume specifications. Multiple volume names
can be connected by “+” sign.

    xpd:mlx5\_0.1/260535867639876.9:e58c053d-ac6b-4973-9722-cf932843fe4e[+mlx...]

A module specification consists of several parts. Caller provides a
device handle, the target serial number, the target link number, the
volume UUID, The convention for specifying a KDSA volume uses the
following format:

    [localdevice[.localport]/][serialnumber[.linknum]:]volumeid

where the square brackets indicate optional parts of the volume
connection specification. Thus, a volumeid is nominally sufficient to
specify a desired volume, and one can then optionally additionally
specify the serial number of the XPD with optional link number, and/or
one can optionally specify the local device to use with optional local
port number. The convention for specifying multiple KDSA volumes to
stripe together uses the following format:

    volume_specifier[+volume_specifier[+volume_specifier[+...]]][@parameter]

where the square brackets indicate optional parts of the aggregated
connection specification. Thus, a single volume connection specification
is sufficient for a full connection specifier, and one can then
optionally specify additional volume specifiers to aggregate, using the
plus sign as a separator. The user may also additionally specify
parameters for the aggregation, using the “at sign,” a single character
as a parameter identifier, and the parameter value.

    {
    "type": "KDSA",
    "id": "p1",
    "target": "This is the XPD connection string",
    "max-threads-per-node" : 0,
    "max-fragment-size" : 1048,
    "accessibility" : "global"
    }

##### POSIX

Path to a directory, e.g., /home/user/data/

    {
     "type": "POSIX",
     "id": "p2",
     "target": "./_posix2"
    }

##### S3

Bucket name prefix (at least 5 characters long)

<div id="tab:s3_params">

| Parameter          | Type    | Default   |          | Description    |
|:-------------------|:--------|:----------|:---------|:---------------|
| host               | string  | (not set) | required | (S3 only) TODO |
| secret-key         | string  | (not set) | required | (S3 only) TODO |
| access-key         | string  | (not set) | required | (S3 only) TODO |
| locationConstraint | string  | (not set) | required | (S3 only) TODO |
| authRegion         | string  | (not set) | required | (S3 only) TODO |
| timeout            | integer | (not set) | required | (S3 only) TODO |
| s3-compatible      | integer | (not set) | required | (S3 only) TODO |
| use-ssl            | integer | 0         | required | (S3 only) TODO |

S3 parameters

</div>

TODO example

##### WOS

TODO

    {
    "type": "WOS",
    "id": "w1",
    "target": "host=192.168.80.33;policy=test;",
    "max-threads-per-node" : 1,
    "max-fragment-size" : 1073741825,
    "max-global-threads" : 1,
    "accessibility" : "global"
    }

##### Parameter: performance-model

<div id="tab:dyn_perf_model_conf_params">

| Parameter  | Type  | Default | Domain   |          | Description |
|:-----------|:------|:--------|:---------|:---------|:------------|
| latency    | float | 0.0     | &gt;=0.0 | optional | seconds     |
| throughput | float | 0.0     | &gt;0.0  | optional | MiB/s       |

Dynamic performance model parameters

</div>

<div id="tab:gen_perf_model_conf_params">

| Parameter  | Type    | Default | Domain   |          | Description         |
|:-----------|:--------|:--------|:---------|:---------|:--------------------|
| latency    | float   | 0.0     | &gt;=0.0 | optional | Latency in seconds  |
| throughput | float   | 0.0     | &gt;0.0  | optional | Throughput in MiB/s |
| size       | integer | 0       | &gt;0    | optional | TODO                |
| period     | float   | 0.0     | &gt;0.0  | optional | TODO                |
| alpha      | float   | 0.0     | \[0.0,   | 1.0)     | optional TODO       |

Generic performance model parameters

</div>

##### Parameter: max-threads-per-node

TODO

##### Parameter: write-stream-blocksize

TODO

##### Parameter: max-global-threads

TODO

##### Parameter: accessibility

<div id="tab:accessibility">

| Method | Description |
|:-------|:------------|
| global | TODO        |
| local  | TODO        |

Accessibility

</div>

##### Parameter: max-fragment-size

The amount of data that may be written into a single fragment. The
amount is given in bytes.

##### Parameter: fragmentation-method

A string identifying the algorithm to use to split a chunk of data into
fragments. Legal values are:

<div id="tab:frag_methods">

| Method     | Description                                                                                                                                                                                                                                                                                                                                                                                                                                                      |
|:-----------|:-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| contiguous | This algorithm tries to form fragments that are scattered across memory as little as possible. As such, it is expected to yield the best possible write performance. However, if a transposition is performed when reading the data back, performance may be poor. Splitting a dataspace of size (50, 80, 100) into fragments of 2000 entries results in fragments of size (1, 20, 100).                                                                         |
| equalized  | This algorithm tries to form fragments that have a similar size in all dimensions. As such, it is expected to perform equally well with all kinds of read requests, but it tends to write scattered data to disk which has to be sequentialized first, imposing a performance penalty on the write side. Splitting a dataspace of size (50, 80, 100) into fragments of at most 2000 entries results in fragments of sizes between (10, 11, 11) and (10, 12, 12). |

Fragmentation methods

</div>

### Metadata

<div id="tab:metadata_params">

| Parameter | Type   | Default   | Description     |
|:----------|:-------|:----------|:----------------|
| type      | string | (not set) | required TODO   |
| id        | string | (not set) | (not used) TODO |

Metadata parameters

</div>

# Installation Instructions for Mistral

Mistral (HLRE-3) is the supercomputer installed at DKRZ in 2015/2016.
This guide documents installation procedures to build prerequisites as
well as the prototype code base for development purposes.

## Satisfying Requirements

Mistral offers users a number of software packages via the module
command (man module). While large parts of ESDM will build with the
standard tools available on the platform, it is still recommended to use
Spack <https://spack.readthedocs.io/en/latest/index.html> to build,
manage and install dependencies.

## Download and enable Spack

    git clone --depth=2 https://github.com/spack/spack.git spack
    . spack/share/spack/setup-env.sh

## Set a gcc version to be used

gccver=7.3.0

## Install Dependencies

    spack install gcc@$gccver +binutils
    spack install autoconf%gcc@$gccver
    spack install openmpi%gcc@$gccver gettext%gcc@$gccver
    spack install jansson%gcc@$gccver glib%gcc@$gccver

## Load Dependencies

    spack load gcc@$gccver
    spack load -r autoconf%gcc@$gccver
    spack load -r libtool%gcc@$gccver

    spack load -r openmpi%gcc@$gccver
    spack load -r jansson%gcc@$gccver
    spack load -r glib%gcc@$gccver

### HDF5 with Virtual Object Layer Support

Assuming all prerequisites have been installed and tested a development
branch of HDF5 can be build as follows.

## Ensure environment is aware of dependencies installed using spack and dev-env

## Download HDF5 with VOL support

    svn checkout https://svn.hdfgroup.org/hdf5/features/vol/

## Configure and build HDF5

    cd vol
    ./autogen.sh
    export CC=mpicc         ### parallel features require mpicc
    ../configure \
        --prefix=$prefix \
        --enable-parallel \
        --enable-build-mode=debug \
        --enable-hl \
        --enable-shared
    make -j
    make test
    make install

### NetCDF with Support for ESDM VOL Plugin

Assuming all prerequisites have been installed and tested a patched
version of NetCDF to enable/disable ESDM VOL Plugin support can be build
as follows.

## Ensure environment is aware of dependencies installed using spack and dev-env

## Downlaod NetCDF source

version=4.5.0

    wget ftp://ftp.unidata.ucar.edu/pub/netcdf/netcdf-$version.tar.gz
    cd netcdf-$version

## Apply patches to allow enabling/disabling plugin

    patch -b --verbose $PWD/libsrc4/nc4file.c $ESDM_REPO_ROOT/dev/netcdf4-libsrc4-nc4file-c.patch
    patch -b --verbose $PWD/include/netcdf.h $ESDM_REPO_ROOT/dev/netcdf4-include-netcdf-h.patch

## Configure and build

    export CC=mpicc
    mkdir build && cd build
    cmake \
      -DCMAKE_PREFIX_PATH=$prefix \
      -DCMAKE_INSTALL_PREFIX:PATH=$prefix \
      -DCMAKE_C_FLAGS=-L$prefix/lib/ \
      -DENABLE_PARALLEL4=ON ..
    make -j
    make test
    make install

### Building ESDM Prototype

Assuming all prerequisites have been installed and tested ESDM can be
configured and build as follows.

## Ensure environment is aware of dependencies installed using spack and dev-env

    cd $ESDM_REPO_ROOT

## Configure and build ESDM

    ./configure --enable-hdf5 --enable-netcdf4 --debug
    cd build
    make

    make test

# Use Cases

This part of the documenation presents a number of use-cases for a
middleware to handle earth system data. The description is organized as
follows:

-   common workloads in climate and weather forecasts (anchor link)

-   involved stakeholders/actors and systems (anchor link)

-   and the actual use cases (anchor link)

The ese cases can extend each other, and are generally constructed in a
way that they are not limited to the ESDM but also apply to similar
middleware. The use of backends is kept abstract where possible, so that
in principle implementations can be swapped with only minor semantic
changes to the sequence of events.

##  Workloads in climate and weather

The climate and weather forecast communities have their characteristic
workflows and objectives, but also share a variety of methods and tools
(e.g., the ICON model is used and developed together by climate and
weather scientists). This section briefly collects and groups the
data-related high-level use-cases by community and the motivation for
them.

Numerical weather prediction focuses on the production of a short-time
forecast based on initial sensor (satellite) data and generates derived
data products for certain end users (e.g., weather forecast for the
general public or military). As the compute capabilities and
requirements for users increase, new services are added or existing
services are adapted. Climate predictions run for long time periods and
may involve complex workflows to compute derived information such as
monthly mean or to identify certain patterns in the forecasted data such
as tsunamis.

In the following, a list of characteristic high-level workloads and
use-cases that are typically performed per community is given. These
use-cases resemble what a user/scientist usually has in mind when
dealing with NWP and climate simulation; there are several supportive
use-cases from the perspective of the data center that will be discussed
later.

### NWP

-   Data ingestion: Store incoming stream of observations from
    satellites, radar, weather stations and ships.

-   Pre-Processing: Cleans, adjusts observation data and then transforms
    it to the data format used as initial condition for the prediction.
    For example, insufficient sampling makes pre-processing necessary so
    models can be initialized correctly.

-   Now Casting (0-6h): Precise prediction of the weather in the near
    future. Uses satellite data and data from weather stations,
    extrapolates radar echos.

-   Numeric Model Forecasts (0-10+ Days): Run a numerical model to
    predict the weather for the next few days. Typically, multiple
    models (ensembles) are run with some perturbed input data. The model
    proceeds usually as follows: 1) Read-Phase to initialize
    simulation; 2) create a periodic snapshots (write) for the model
    time, e.g., every hour.

-   Post-Processing: create data products that may be used for multiple
    purposes.

    -   for Now Casting: multi sensor integration, classification,
        ensembles, impact models

    -   for Numeric Model Forecasts: statistical interpretation of
        ensembles, model-combination

    -   generation of data products like GRIB files as service for
        customers of weather forecast data

-   Visualizations: Create fancy presentations of the future weather;
    this is usually part of the post-processing.

### Climate

Many use cases in climate are very similar:

-   Pre-Processing: Similar to the NWP use case.

-   Forecasting with Climate Models: Similar to the NWP use case, with
    the following differences:

    -   The periodic snapshots (write) uses variable depending
        frequencies, e.g., some variables are written out with higher
        frequencies than others

-   Post-Processing: create data products that are useful, e.g., run
    CDOs (Climate Data Operations) to generate averages for certain
    regions. The performed post-processing depends on the task the
    scientist has in mind. While at runtime of the model some products
    are clear and may be used to diagnose the simulation run itself,
    later scientists might be interested to run additional
    post-processing to look for new phenomena.

-   Dynamic visualization: use interactive tools to search for
    interesting patterns. Tools such as VTK and Paraview are used.

-   Archive data: The model results are stored on a long-term archive.
    They may be used for later analysis – often at other sites and by
    another user, e.g., to search for some interesting pattern, or to
    serve as input data for localized higher-resolution models. Also it
    supports reproduceability of research.

## Stakeholders and Systems

## List of Use-Cases

The use cases are organized as one document per use case. The available
use cases are:

-   Independent Write

-   Independent Read

-   Simulation

-   Pre/Post Processing

...

# Styleguide for ESDM development

This document describes the style guide to use in the code.

## General rules

-   Do not break the line after a fixed number of characters as this is
    the duty of the editor to use some softwrap.

    -   You may use a line wrap, if that increases readability (see
        example below).

-   Use two characters for indentation per level

-   Documentation with Doxygen needs to be added on the header files

-   Ensure that the code does not produce WARNINGS

-   Export only the functions to the user that is needed by the user

-   The private (module-internal) interface is defined in **internal**.h

## Naming conventions

-   use lower case for the public interface

-   functions for users provided by ESDM start with esdm\_

-   auxiliary functions that are used internally start with ea\_ (ESDM
    auxiliary) and shall be defined inside esdm-internal.h

## Example Code

    //First add standard libraries
    #include <stdio.h>
    #include <stdlib.h>

    // Add an empty line before adding any ESDM include file
    #include <esdm-internal.h>

    struct x_t{
      int a;
      int b;
      int *p;
    };

    // needs always to be split separately, 
    // to allow it to coexist in a public header file
    typedef struct x_t x_t; 

    int testfunc(int a){
      {
        // Additional basic block
      }
      if (a == 5){

      }else{

      }

      // allocate variables as late as possible, 
      // such that we can see when it is needed and what it does.
      int ret;

      switch(a){
        case(5):{
          break;
        }case(2):{

        }
        default:{
          xxx
        }
      }

      return 0;
    }

# Developers Guide

This document aims to give a quick conceptual overview of the API that
is available to programs which link directly to ESDM. It is not meant to
be exhaustive, but rather to give enough background information so that
reading the documentation of the ESDM API becomes easy. The intended
audience of this guide are developers of scientific applications who
wish to benefit of the full potential of the ESDM API.

Readers who are interested in using an ESDM file system with a NetCDF
based application should read the users guide instead. Unfortunately,
there is no guide for ESDM backend developers and ESDM core contributors
yet.

## Internal Data Model

The first thing to understand is the data model that is used by ESDM.
This data model is very similar to that employed by NetCDF, but it does
add some abstractions, and is described in this section.

### Dataspace

A dataspace describes how data is stored in memory, it is basically a
mapping of a logical dataspace to sequential bytes in memory. All data
handled by ESDM is associated with a dataspace, mostly the dataspaces
are user provided. Several copies of the same data may use distinct
dataspaces, and ESDM allows users to transform data from one layout to
another by providing a source and a destination dataspace
(‘esdm\_dataspace\_copy\_data()’). User code needs to provide a
dataspace when it defines a variable (here the data layout part is
irrelevant), when it stores data, and when it reads data.

The logical dataspace is always a multidimensional, axis aligned box in
ESDM. As such, a dataspace consists of the following information:

-   the dimension count

-   start and end coordinates for each axis

-   a datatype describing a single value within the multidimensional
    array

-   (data serialization information)

The data serialization information is usually implicit, ESDM will simply
assume C multidimensional array layout. Fortran programs will need to
set the data serialization information explicitly to match the inverse
dimension order of Fortran. The data serialization information
(‘stride’) can also achieve unorthodox effects like arrays with holes,
or to replicate a single 2D slice along a third axis.

#### Creating and Destroying Dataspaces

ESDM provides two distinct mechanisms to create a dataspace: A generic
API which allocates the dataspace on the heap, and an API to quickly
create a throw-away dataspace on the stack.

##### The Generic API

The functions used to create dataspaces are:

-   ‘esdm\_dataspace\_create()’ Constructs a dataspace with a given
    dimension count, size and datatype. This assumes that the hypercube
    starts at (0, 0, ..., 0) and C data layout.

-   ‘esdm\_dataspace\_create\_full()’ Like ‘esdm\_dataspace\_create(),’
    but allows the user to specify start coordinates.

-   ‘esdm\_dataspace\_copy()’ Copy constructor.

-   ‘esdm\_dataspace\_subspace()’ Create a dataspace that contains a
    subset of the logical space of its parent dataspace. This copies the
    datatype and assumes C data layout for the subspace. If this is not
    desireable, follow up with a call to
    ‘esdm\_dataspace\_copyDatalayout().’

-   ‘esdm\_dataspace\_makeContiguous()’ Create a dataspace with the same
    logical space and datatype, but which uses the C data layout.

All these functions return a pointer to a new dataspace \*\*which must
be destroyed with a call to ‘esdm\_dataspace\_destroy()’\*\*.

Data layout can only be set explicitly after a dataspace has been
created. This is done by a call to ‘esdm\_dataspace\_set\_stride()’ or
to ‘esdm\_dataspace\_copyDatalayout().’ The first allows the user to
specify any regular data layout, including, but not limited to Fortran
data layout. The later assumes that the dataspace will be used to access
the same data buffer as the dataspace from which the data layout is
copied. As such, ‘esdm\_dataspace\_copyDatalayout()’ is very convenient
for data subseting operations.

##### The Simple API

For convenience and performance reasons, ESDM provides a set of
preprocessor macros that allocate a dataspace on the stack. Macros are
provided for 1D, 2D and 3D dataspaces, and come in variants that either
assume start coordinates at the origin of the coordinate system, or
allow the offset to be specified explicitly. The macros without explicit
start coordinates are:

    esdm_dataspace_1d()
    esdm_dataspace_2d()
    esdm_dataspace_3d()

The macros that take start coordinates simply add an "o" to the end of
the name:

    esdm_dataspace_1do()
    esdm_dataspace_2do()
    esdm_dataspace_3do()

The result of these macros is a value of type ‘esdm\_simple\_dspace\_t’
which is a struct containing a single member ‘ptr’ which contains the
pointer value that can subsequently be used in all ESDM calls that
require a dataspace. I.e, a typical usage might be:

    esdm_simple_dspace_t region = esdm_dataspace_2do(x, width, y, height, type);
    esdm_read(dataset, buffer, region.ptr);

As the esdm\_simple\_dspace\_t lives on the stack, \*\*it must not be
destroyed with ‘esdm\_dataspace\_destroy()’\*\*. It simply ceases to
exists when the surrounding block exits.

### Dataset

A dataset in ESDM is what a variable is in NetCDF or HDF5. Each dataset
is associated with a dataspace that describes the logical extends of the
data, and it acts as a container for data that is written into this
logical space.

There is no requirement to fill the entire logical space with data.
Normally, reading nonexistent data results in an error. However, a
dataset can also be associated with a fill value to handle data with
holes seamlessly.

There is also no requirement to write nonoverlapping data. When writes
overlap, ESDM will assume that both writes place the same data in the
overlapping area. If this condition does not hold, there is no guarantee
which data will be returned on a read.

In addition to the data and the logical space, datasets can also contain
a number of attributes. Like attributes in NetCDF, these are meant to
associate metadata with a dataset.

User code can either create a dataset with ‘esdm\_dataset\_create()’ or
look up an existing dataset from a container with
‘esdm\_dataset\_open().’ In either case, the reference to the dataset
must later be dropped by a call to ‘esdm\_dataset\_close().’ A dataset
can also be deleted with a call to ‘esdm\_dataset\_delete()’ which will
remove its data from the file system, as well as its name and link from
its container.

### Container

Containers provide a means to organize and retrieve datasets. When a
dataset is created, it is added to a container and associated with a
name for later retrieval.

Like datasets, containers are created with ‘esdm\_container\_create()’
or looked up from the file system with ‘esdm\_container\_open().’ Also
like datasets, containers need to be closed with
‘esdm\_container\_close()’ when their reference is not needed anymore.
Closing a container requires closing all datasets it contains first.
‘esdm\_container\_delete()’ removes a container from the file system.

### Grid

The grid abstraction exists for performance reasons only: While it is
possible to think of a dataset as a set of possibly overlapping chunks
of data, it is surprisingly hard to determine minimal sets of chunks to
satisfy a read request. User code, on the other hand, generally does not
use overlapping chunks of data. Instead, user code can be assumed to
work on (semi-)regular nonoverlapping chunks of data. Passing this
chunking information to ESDM allows the library to make good decisions
much faster.

Grids also allow user code to inquire how the data is available on disk,
allowing consumer code to iterate over complete and nonoverlapping sets
of data chunks in the most efficient fashion.

To work with grids, the header ‘esdm-grid.h’ must be included.

Like a dataspace, a grid covers an axis aligned hyperbox in the logical
space. This space is called the grid’s domain, and it is defined when a
grid is created with ‘esdm\_grid\_create(),’
‘esdm\_grid\_createSimple()’ allows omitting the start coordinates to
use the origin as one corner of the hyperbox.

Once a grid has been created, its axes can be subdivided individually by
calls to ‘esdm\_grid\_subdivide().’ This allows the user to specify all
the bounds for an axis explicitly. In many contexts, however, it will be
simpler to use ‘esdm\_grid\_subdivideFixed()’ or
‘esdm\_grid\_subdivideFlexible()’ which instruct ESDM to generate bounds
in a regular way. Fixed subdivision will produce intervals of a given
size, flexible subdivision instructs ESDM to generate a specific number
of intervals of similar size.

After the axis of a grid have been defined, individual grid cells may be
turned into subgrids via ‘esdm\_grid\_createSubgrid().’ After this, the
axes of the parent grid are fixed and the subdivision calls cannot be
used anymore. The subgrid, on the other hand, is a newly created grid
with the parent grids cell bounds as its domain. Usually, user code will
follow up with calls to ‘esdm\_grid\_subdivide\*()’ on the subgrid to
define its axes. Subgrids may be constructed recursively to any depth.

This subgrid feature is useful to define grids with semi-regular
decompositions: For instance, an image may be decomposed into stripes,
which are themselves decomposed into rectangles, but the rectangle
bounds of one stripe do not match those of another stripe. Such
semi-regular decompositions are a common result of load balancing of
earth system simulations.

Once a grids structure has been defined fully, it can be used to
read/write data via ‘esdm\_read\_grid()’ and ‘esdm\_write\_grid().’
Parallel applications will want to distribute the grid to all involved
processes first by calling ‘esdm\_mpi\_grid\_bcast()’ (include
‘esdm-mpi.h’ for this). Both, using a grid for input/output and
communicating it over MPI will fix the grids structure, prohibiting
future calls to subdivide axes or create subgrids.

It is possible to iterate over all (sub-)cells of a grid. This is done
using an iterator, the three relevant functions are
‘esdm\_gridIterator\_create(),’ ‘esdm\_gridIterator\_next()’ and
‘esdm\_gridIterator\_destroy().’ This is meant to be used by readers
which inquire an available grid from a dataset using
‘esdm\_dataset\_grids().’ This method of reading avoids any cropping or
stitching together of data chunks within ESDM, delivering the best
possible performance.

Grids always remain in possession of their dataspace. Consequently, it
is not necessary to dispose of them explicitly. However, closing a
dataspace invalidates all associated (sub-)grids.

## Usage Examples

Learning usage of an API is easiest by seeing it in action in concrete
examples. As such, this section provides four relatively basic examples
of how the ESDM API is supposed to be used, which nevertheless cover all
the required core functionality.

### Basic Writing

The simplest way to write a grey scale image to ESDM is as follows:

    //assume image data stored in either
    uint16_t imageBuffer[height][width];
    uint16_t (*imageBuffer)[width] = malloc(height*sizeof*imageBuffer);

    //initialize ESDM
    esdm_status result = esdm_init();
    assert(result == ESDM_SUCCESS);

    //create the container, dataspace, and dataset
    esdm_container_t* container;
    //pass true to allow overwriting of an existing container
    result = esdm_container_create("path/to/container", false, &container); 
    assert(result == ESDM_SUCCESS);
    //the data is 2D and consists of uint64_t values
    esdm_simple_dspace_t space = 
      esdm_dataspace_2d(height, width, SMD_TYPE_UINT16);  esdm_dataset_t* dataset;
    result = esdm_dataset_create(container, "myImage", space.ptr, &dataset);
    assert(result == ESDM_SUCCESS);

    //write the data
    result = esdm_write(dataset, imageBuffer, space.ptr);
    assert(result == ESDM_SUCCESS);

    //cleanup
    result = esdm_dataset_close(dataset);
    assert(result == ESDM_SUCCESS);
    result = esdm_container_close(container);
    assert(result == ESDM_SUCCESS);

    //bring down ESDM
    result = esdm_finalize();
    assert(result == ESDM_SUCCESS);

In this example, the same dataspace is used to create the dataset and to
write the data, writing all the data in one large piece. This is not
necessary, the dataspaces that are passed to ‘esdm\_write()’ may be
smaller than the dataset, calling ‘esdm\_write()’ as many times as
required to write the entire data.

### Grid Based Writing

When using grid based writing, the creation of the container and the
dataset is exactly the same. The creation of the grid, however is added
explicitly. In this case, we are going slice a 3D dataspace into 10
slices of similar size along the z axis, and into rows of 256 lines
along the y axis:

    // define the grid
    esdm_grid_t* grid;
    result = esdm_grid_createSimple(dataset, 3, (int64_t[3]){depth, height, width}, &grid);
    assert(result == ESDM_SUCCESS);
    // the last parameter allows the last interval to be smaller than 256 lines
    result = esdm_grid_subdivideFixed(grid, 1, 256, true);  
    assert(result == ESDM_SUCCESS);
    result = esdm_grid_subdivideFlexible(grid, 0, 10);
    assert(result == ESDM_SUCCESS);

    for(int64_t z0 = 0, slice = 0, curDepth; z0 < depth; z0 += curDepth, slice++) {
        // inquire the depth of the current slice
        // use of esdm_grid_subdivideFlexible() generally 
        // requires use of a grid bounds inquiry function
        int64_t z1;
        result = esdm_grid_getBound(grid, 0, slice + 1, &z1);
        assert(result == ESDM_SUCCESS);
        curDepth = z1 - z0;

        for(int64_t row = 0; row*256 < height; row++) {
            // compute the height of the current row
            // we can calculate this ourselves as we have used esdm_grid_subdivideFixed()
            int64_t height = (row < height/256 ? 256 : height - row*256);

            // set contents of dataBuffer

            // use the grid to write one chunk of data, the grid knows 
            // to which dataset it belongs
            result = esdm_write_grid(
                grid, 
                esdm_dataspace_3do(z0, curDepth, row*256, height, 0, width, SMD_TYPE_DOUBLE).ptr, 
                dataBuffer);
            assert(result == ESDM_SUCCESS);
        }
    }

    // no need to cleanup the grid, it belongs to the dataset and 
    // will be disposed off when the dataset is closed

### Simple Reading

Reading data is very similar to writing it. Nevertheless, a simple
example is given to read an entire dataset in one piece:

    // open the container and dataset, and inquire the dataspace
    esdm_container_t* container;
    result = esdm_container_open("path/to/container", ESDM_MODE_FLAG_READ, &container);
    assert(result == ESDM_SUCCESS);
    esdm_dataset_t* dataset;
    result = esdm_dataset_open(container, "myDataset", ESDM_MODE_FLAG_READ, &dataset);
    assert(result == ESDM_SUCCESS);
    esdm_dataspace_t* dataspace;
    // this returns a reference to the internal dataspace, do not destroy or modify it
    esdm_dataset_get_dataspace(dataset, &dataspace);  

    // allocate a buffer large enough to hold the data and generate a dataspace for it
    // the buffer will be in contiguous C data layout
    result = esdm_dataspace_makeContiguous(dataspace, &dataspace);  
    assert(result == ESDM_SUCCESS);
    int64_t bufferSize = esdm_dataspace_total_bytes(dataspace);
    void* buffer = malloc(bufferSize);

    // read the data
    result = esdm_read(dataset, buffer, dataspace);
    assert(result == ESDM_SUCCESS);

    // do stuff with buffer and dataspace

    // cleanup
    // esdm_dataspace_makeContiguous() creates a new dataspace
    result = esdm_dataspace_destroy(dataspace); 
    assert(result == ESDM_SUCCESS);
    result = esdm_dataset_close(dataset);
    assert(result == ESDM_SUCCESS);
    result = esdm_container_close(container);
    assert(result == ESDM_SUCCESS);

### Grid Based Reading

Reading an entire dataset as a single chunk is generally a really bad
idea. Datasets, especially those generated by earth system models, may
be huge, many times larger than the available main memory. Reading a
dataset in the form of reader defined chunks is possible with
‘esdm\_read(),’ but not necessarily efficient. The chunks on disk may
not match those which are used to read, requiring ‘esdm\_read()’ to

-   read multiple chunks from disk and stitch them together,

-   and to read more data from disk than is actually required.

If the dataset has been written using a grid, this grid can be recovered
to inform the reading process of the actual data layout on disk:

        // open the container and dataset, and inquire the available grids
        esdm_container_t* container;
        result = esdm_container_open("path/to/container", ESDM_MODE_FLAG_READ, &container);
        assert(result == ESDM_SUCCESS);
        esdm_dataset_t* dataset;
        result = esdm_dataset_open(container, "myDataset", ESDM_MODE_FLAG_READ, &dataset);
        assert(result == ESDM_SUCCESS);
        int64_t gridCount;
        esdm_grid_t** grids;
        result = esdm_dataset_grids(dataset, &gridCount, &grids);
        assert(result == ESDM_SUCCESS);

        // select a grid, here we just use the first one
        assert(gridCount >= 1);
        esdm_grid_t* grid = grids[0];
        free(grids);  //we are responsible to free this array

        // iterate over the data, reading the data one stored chunk at a time
        esdm_gridIterator_t* iterator;
        result = esdm_gridIterator_create(grid, &iterator);
        assert(result == ESDM_SUCCESS);
        while(true) {
            esdm_dataspace_t* cellSpace;
            result = esdm_gridIterator_next(&iterator, 1, &cellSpace);
            assert(result == ESDM_SUCCESS);

            if(!iterator) break;

            // allocate a buffer large enough to hold the data
            int64_t bufferSize = esdm_dataspace_total_bytes(cellSpace);
            void* buffer = malloc(bufferSize);

            // read the data
            result = esdm_read_grid(grid, cellSpace, buffer);
            assert(result == ESDM_SUCCESS);

            // do stuff with buffer and cellSpace

            // cleanup
            free(buffer);
            result = esdm_dataspace_destroy(cellSpace);
            assert(result == ESDM_SUCCESS);
        }

        // cleanup
        // no cleanup necessary for the iterator, it has already been destroyed 
        // by esdm_gridIterator_next()
        result = esdm_dataset_close(dataset);
        assert(result == ESDM_SUCCESS);
        result = esdm_container_close(container);
        assert(result == ESDM_SUCCESS);

# Callgraph for accessing metadata

Guiding question:

-   How and when do we fetch metadata in the read path?

-   When do we serialize metadata to JSON in the write path?

## General responsibility

-   Container holds references to datasets (under different name
    possibly)

-   Dataset holds information about the dataset itself:

    -   User-defined attributes (scientific metadata) =&gt; it is not
        know a-priori what that is

    -   Technical attributes (some may be optional) =&gt; well known how
        they look like

    -   Fragment information is directly inlined as part of datasets

-   Fragments

    -   Information about their shape, backend plugin ID,
        backend-specific options (unknown to us)

## Write

Applies for datasets, containers, fragments:

-   All data is kept in appropriate structures in main memory

-   Serialize to JSON just before calling the metadata backend to store
    the metadata (and free JSON afterwards)

-   Backend communicates via JSON to ESDM layer

Callgraph from user perspective:

-   c = container\_create("name")

-   d = dataset\_create(c, "dset")

-   write(d) creates fragments and attaches the metadata to the dataset

-   dataset\_commit(d) =&gt; make the dataset persistent, also write the
    dataset + fragment metadata

-   container\_commit(c) =&gt; makes the container persistent TODO:
    check that the right version of data is linked to it.

-   dataset\_destroy(d)

-   container\_destroy(c)

## Read

Applies for datasets, containers, fragments:

-   All data is kept in appropriate structures in main memory AND
    fetched when the data is queried initially

-   De-serialized from JSON at the earliest convenience in the ESDM
    layer

-   Backend communicates via JSON to ESDM layer

Callgraph from user perspective:

-   c = container\_open("name") &lt;= here we read the metadata for
    "name" and generate appropriate structures

-   d = dataset\_open("dset") &lt;= here we read the metadata for "dset"
    and generate appropriate structures

-   read(d)

-   dataset\_destroy(d)

-   container\_destroy(c)

Alternative workflow with unkown dsets:

-   c = container\_open("name") &lt;= here we read the metadata for
    "name" and generate appropriate structures

-   it = dataset\_iterator()

-   for x in it:

    -   dataset\_iterator\_dataset(x)

-   read(d)

-   dataset\_destroy(d)

-   container\_destroy(c)

# Grid deduplication

## Requirements

-   The user must be allowed to create throw-away grids for reading

-   It must be avoided that there are several fragments with the exact
    same shape

-   It must be avoided that there are several grids with the exact same
    axis and subgrid structure

-   → deduplication must happen on two distinct levels: the fragment
    level and the grid level

-   It is not possible to reference count grids or fragments, as these
    objects contain a reference to their owning dataset, and thus must
    not survive its destruction

    -   → deduplication must rely on proxy objects that reference the
        already existing objects

-   User code can hold pointers to subgrids

    -   → grids cannot be replaced with proxy objects, they must be
        turned into proxy objects

    -   → grids that are turned into proxy objects must retain their
        subgrid structure to ensure proper destruction

-   A fragment cannot be matched with a subgrid, and a subgrid cannot be
    matched with another subgrid that differs in one of the axes or
    subgrids

    -   → deduplication cannot happen while the grid is still in fixed
        axes state

    -   → esdm\_read\_grid() and esdm\_write\_grid() should be defined
        to put a grid into fixed structure state, allowing deduplication
        on their first call

## Implementation

-   Fragments are managed in a centralized container (hashtable keyed by
    their extends), and referenced by the grids.

    -   A count of referencing grids can be added if necessary. This is
        not a general ref count, the dataset still owns the fragments
        and destructs them in its destructor. The grid count would allow
        the dataset to delete a fragment when its containing grids get
        deleted.

    -   The fragment references in the grids remain plain pointers, but
        the grids loose their ownership over the fragments.

    -   The MPI code that handles grids must be expanded to also
        communicate fragment information explicitly.

    -   As a side effect, this also allows the dataset to unload any
        fragments to prevent run-away memory consumption.

-   Grids contain a delegate pointer.

    -   All grid methods must first resolve any delegate chain.

    -   The delegate pointer is set when the grid is first touched with
        an I/O or MPI call and detected to be structurally identical to
        an existing grid.

    -   When the delegate pointer is set, the delegate pointers of all
        subgrids are also set.

    -   There are two possible implementations for managing grid proxy
        objects:

        -   The proxy grid’s structure data remains valid (axes and cell
            matrix), the proxy’s destructor recursively destructs its
            subgrid proxies.

        -   All existing grids are managed via a flat list of grids
            within the dataset, and grids that become proxies get their
            axis and matrix data deleted immediately.

## Roadmap for Implementation

1.  Create a hash function that works on hypercubes and offset/size
    arrays.

2.  Centralize the storage of fragments in a hash table. This
    deduplicates fragments, and takes fragment ownership away from
    grids. This will break the MPI code.

3.  Fix the MPI interface by communicating fragment metadata separately
    from grid metadata.

4.  Centralize the storage of grids in the dataset. The dataset should
    have separate lists for complete top-level grids, incomplete
    top-level grids, and subgrids.

5.  Implement delegates for grids.

6.  Implement the grid matching machinery to create the delegates.

SUMMARY

# Related Work

VOL in general

-   <https://svn.hdfgroup.org/hdf5doc/trunk/RFCs/HDF5/VOL/RFC/RFC_VOL.pdf>

-   <https://svn.hdfgroup.org/hdf5doc/trunk/RFCs/HDF5/VOL/user_guide/main.pdf>

-   <https://svn.hdfgroup.org/hdf5doc/trunk/RFCs/HDF5/VOL/developer_guide/main.pdf>

[1] A entity of the domain model such as a car could be mapped to one or
several objects.

[2] Taylor et al (2012): CMIP5 Data Reference Syntax (DRS) and
Controlled Vocabularies.
