# the SOURCE definiton lets move your makefile to another position
CONFIG 			= LIBRARY

# set pathes to the correct directories
SRC_DIR			= ../../../../source/Lib/TLibEncoder
INC_DIR			= ../../../../source/Lib/TLibEncoder
LIB_DIR			= ../../../../lib
BIN_DIR			= ../../../../bin

SRC_DIR1		=
SRC_DIR2		=
SRC_DIR3		=
SRC_DIR4		=


USER_INC_DIRS	= -I$(SRC_DIR)
USER_LIB_DIRS	=

# intermediate directory for object files
OBJ_DIR			= ./objects

# the library name
PRJ_NAME	 	= TLibEncoder

# version information
MAJOR_VER		= 0
MINOR_VER		= 1
VER				= $(MAJOR_VER).$(MINOR_VER)

# defines to set
DEFS			= -DMSYS_LINUX

# set objects
OBJS          	= \
			$(OBJ_DIR)/TEncAnalyze.o \
			$(OBJ_DIR)/TEncCu.o \
			$(OBJ_DIR)/TEncEntropy.o \
			$(OBJ_DIR)/TEncLoopFilter.o \
			$(OBJ_DIR)/TEncGOP.o \
			$(OBJ_DIR)/TEncSbac.o \
			$(OBJ_DIR)/TEncSearch.o \
			$(OBJ_DIR)/TEncPicture.o \
			$(OBJ_DIR)/TEncTop.o \

LIBS				= -lpthread

DEBUG_LIBS			=
RELEASE_LIBS		=

STAT_LIBS			=
DYN_LIBS			= -ldl

# the libraries to link with
STAT_DEBUG_LIBS		=
STAT_RELEASE_LIBS	=
DYN_DEBUG_LIBS		=
DYN_RELEASE_LIBS	=

# name of the base makefile
MAKE_FILE_NAME		= ../../common/makefile.base

# include the base makefile
include $(MAKE_FILE_NAME)
