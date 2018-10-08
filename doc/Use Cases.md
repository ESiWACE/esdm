# Use Cases

This part of the documenation presents a number of use-cases for a middleware to handle earth system data.
The description is organized as follows:

* common workloads in climate and weather forecasts (anchor link)
* involved stakeholders/actors and systems (anchor link)
* and the actual use cases (anchor link)

The ese cases can extend each other, and are generally constructed in a way that they are not limited to the ESDM but also apply to similar middleware. The use of backends is kept abstract where possible, so that in principle implementations can be swapped with only minor semantic changes to the sequence of events.



##  Workloads in climate and weather

The climate and weather forecast communities have their characteristic workflows and objectives, but also share a variety of methods and tools (e.g., the ICON model is used and developed together by climate and weather scientists).
This section briefly collects and groups the data-related high-level use-cases by community and the motivation for them.

Numerical weather prediction focuses on the production of a short-time forecast based on initial sensor (satellite) data and generates derived data products for certain end users (e.g., weather forecast for the general public or military). 
As the compute capabilities and requirements for users increase, new services are added or existing services are adapted.
Climate predictions run for long time periods and may involve complex workflows to compute derived information such as monthly mean or to identify certain patterns in the forecasted data such as tsunamis.
%before the community settles for large scale ensemble simulations the models go two a continues process of model and performance optimizations.

In the following, a list of characteristic high-level workloads and use-cases that are typically performed per community is given.
These use-cases resemble what a user/scientist usually has in mind when dealing with NWP and climate simulation; there are several supportive use-cases from the perspective of the data center that will be discussed later.

### NWP

* Data ingestion: Store incoming stream of observations from satellites, radar, weather stations and ships.
* Pre-Processing: Cleans, adjusts observation data and then transforms it to the data format used as initial condition for the prediction. For example, insufficient sampling makes pre-processing necessary so models can be initialized correctly.
* Now Casting (0-6h): Precise prediction of the weather in the near future. Uses satellite data and data from weather stations, extrapolates radar echos.
* Numeric Model Forecasts (0-10+ Days): Run a numerical model to predict the weather for the next few days. Typically, multiple models (ensembles) are run with some perturbed input data. 	The model proceeds usually as follows: 1) Read-Phase to initialize simulation; 2) create a periodic snapshots (write) for the model time, e.g., every hour.
*  Post-Processing: create data products that may be used for multiple purposes.
	* for Now Casting: multi sensor integration, classification, ensembles, impact models
	* for Numeric Model Forecasts: statistical interpretation of ensembles, model-combination
	* generation of data products like GRIB files as service for customers of weather forecast data
* Visualizations: Create fancy presentations of the future weather; this is usually part of the post-processing.

### Climate

Many use cases in climate are very similar:

* Pre-Processing: Similar to the NWP use case.
* Forecasting with Climate Models: Similar to the NWP use case, with the following differences:
	* The periodic snapshots (write) uses variable depending frequencies, e.g., some variables are written out with higher frequencies than others
* Post-Processing: create data products that are useful, e.g., run CDOs (Climate Data Operations) to generate averages for certain regions. The performed post-processing depends on the task the scientist has in mind. While at runtime of the model some products are clear and may be used to diagnose the simulation run itself, later scientists might be interested to run additional post-processing to look for new phenomena.
* Dynamic visualization: use interactive tools to search for interesting patterns. Tools such as VTK and  Paraview are used. 
* Archive data: The model results are stored on a long-term archive. They may be used for later analysis --  often at other sites and by another user, e.g., to search for some interesting pattern, or to serve as input data for localized higher-resolution models. Also it supports reproduceability of research.




## Stakeholders and Systems

## List of Use-Cases

The use cases are organized as one document per use case. The available use cases are:

* Independent Write
* Independent Read
* Simulation
* Pre/Post Processing

...
