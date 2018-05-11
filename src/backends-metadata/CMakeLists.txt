SUBDIRS(metadummy)


# Metadata Backends
option(BACKEND_MONGODB "Compile metadata backend for MongoDB support?" OFF)
if(BACKEND_MONGODB)
	message(STATUS "WITH_BACKEND_MONGODB")
	add_definitions(-DESDM_HAS_MONGODB=1)
	SUBDIRS(mongodb)
endif()
