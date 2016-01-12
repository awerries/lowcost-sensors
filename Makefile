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
BIN_DIR = bin

IMU_SRCS := $(I2Csrc) $(IMUsrc)
IMU_OBJS := $(I2Cobj) $(IMUobj)
IMU_INC := -II2Cdev -IMPU6050

MAG_SRCS := $(I2Csrc) $(MAGsrc)
MAG_OBJS := $(I2Cobj) $(MAGobj)
MAG_INC := -II2Cdev -IHMC6343


.PHONY: directories

all: directories imu_reader mag_reader

directories: ${BIN_DIR}

${BIN_DIR}:
	${MKDIR_P} ${BIN_DIR}

imu_reader: $(IMU_OBJS) $(IMUREADsrc)
	$(CXX) $(LDFLAGS) $(IMU_INC) -o bin/imu_reader $(IMUREADsrc) $(IMU_OBJS) $(LDLIBS)

mag_reader: $(MAG_OBJS) $(MAGREADsrc)
	$(CXX) $(LDFLAGS) $(MAG_INC) -o bin/mag_reader $(MAGREADsrc) $(MAG_OBJS) $(LDLIBS)


$(I2Cobj): $(I2Csrc) $(I2Csrc:%.cpp=%.h)

$(IMUobj): $(IMUsrc) $(IMUsrc:%.cpp=%.h)
	$(CXX) $(CPPFLAGS) $(IMU_INC) -c $(IMUsrc) -o $(IMUobj)

$(MAGobj): $(MAGsrc) $(MAGsrc:%.cpp=%.h)
	$(CXX) $(CPPFLAGS) $(MAG_INC) -c $(MAGsrc) -o $(MAGobj)

clean:
	rm -f $(IMU_OBJS) $(MAG_OBJS)
	rm -f bin/imu_reader
	rm -f bin/mag_reader
