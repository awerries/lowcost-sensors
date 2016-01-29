CC = gcc
CXX = g++
CPPFLAGS = -g -Wall -Wpedantic -Wextra
LDFLAGS = -g
LDLIBS = -lbcm2835 -lm
MKDIR_P = mkdir -p
BASEDIR = $(shell pwd)

I2Csrc = I2Cdev/I2Cdev.cpp
I2Cobj = $(I2Csrc:%.cpp=%.o)
IMUsrc = MPU6050/MPU6050.cpp
IMUobj = $(IMUsrc:%.cpp=%.o)
MAGsrc = HMC6343/HMC6343.cpp
MAGobj =$(MAGsrc:%.cpp=%.o)
IMUREADsrc = Readers/imu_reader.cpp
MAGREADsrc = Readers/mag_reader.cpp
MAGCONFIGsrc = Config_Tools/mag_config.cpp
MAGRESETsrc = Config_Tools/reset_mag_offsets.cpp
BIN_DIR = bin

IMU_BIN := $(BIN_DIR)/imu_reader
IMU_SRCS := $(I2Csrc) $(IMUsrc)
IMU_OBJS := $(I2Cobj) $(IMUobj)
IMU_INC := -II2Cdev -IMPU6050

MAG_BIN := $(BIN_DIR)/mag_reader
MAGCONFIG_BIN := $(BIN_DIR)/mag_config
MAGRESET_BIN := $(BIN_DIR)/reset_mag_offsets
MAG_SRCS := $(I2Csrc) $(MAGsrc)
MAG_OBJS := $(I2Cobj) $(MAGobj)
MAG_INC := -II2Cdev -IHMC6343


.PHONY: directories

all: directories $(IMU_BIN) $(MAG_BIN) $(MAGCONFIG_BIN) $(MAGRESET_BIN)

directories: $(BIN_DIR)

$(BIN_DIR):
	$(MKDIR_P) $(BIN_DIR)

$(IMU_BIN): $(IMUREADsrc) $(IMU_OBJS)
	$(CXX) $(LDFLAGS) $(IMU_INC) -o $@ $^ $(LDLIBS)

$(MAG_BIN): $(MAGREADsrc) $(MAG_OBJS) 
	$(CXX) $(LDFLAGS) $(MAG_INC) -o $@ $^ $(LDLIBS)

$(MAGCONFIG_BIN): $(MAGCONFIGsrc) $(MAG_OBJS)
	$(CXX) $(LDFLAGS) $(MAG_INC) -o $@ $^ $(LDLIBS)

$(MAGRESET_BIN): $(MAGRESETsrc) $(MAG_OBJS)
	$(CXX) $(LDFLAGS) $(MAG_INC) -o $@ $^ $(LDLIBS)

$(I2Cobj): $(I2Csrc) $(I2Csrc:%.cpp=%.h)

$(IMUobj): $(IMUsrc) $(IMUsrc:%.cpp=%.h)
	$(CXX) $(CPPFLAGS) $(IMU_INC) -c $< -o $@

$(MAGobj): $(MAGsrc) $(MAGsrc:%.cpp=%.h)
	$(CXX) $(CPPFLAGS) $(MAG_INC) -c $< -o $@

clean:
	rm -f $(IMU_OBJS) $(MAG_OBJS) $(IMU_BIN) $(MAG_BIN) $(MAGCONFIG_BIN)
