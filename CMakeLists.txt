PROJECT (CSM C)


FIND_PROGRAM(HOSTNAME hostname PATHS $ENV{PATH})

IF(HOSTNAME)
	EXEC_PROGRAM(${HOSTNAME} ARGS -s OUTPUT_VARIABLE host)
	SET(local "local-${host}.cmake")
	
	SET(extension "$ENV{CSM_CONF}")
	IF(extension)
		SET(local "local-${host}-${extension}.cmake")
	ENDIF(extension)
	
	IF(EXISTS ${local})
		MESSAGE(STATUS "Including local configuration ${local}.")
		INCLUDE(${local})
	ELSE(EXISTS ${local})
		MESSAGE(STATUS "Create a file ${local} if you want to add options.")
	ENDIF(EXISTS ${local})
ELSE(HOSTNAME)
	MESSAGE(STATUS "Program `hostname` not found. (don't worry)")
ENDIF(HOSTNAME)

ENABLE_TESTING()
SUBDIRS(sm)
