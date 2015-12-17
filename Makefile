CC=gcc
CXX=g++
CPPFLAGS=-g -Wall -Wpedantic -Wextra
LDFLAGS=-g
LDLIBS=-lbcm2835 -lm

I2Clib=I2Cdev
IMUlib=MPU6050

SRCS=$(I2Clib)/I2Cdev.cpp $(IMUlib)/MPU6050.cpp
OBJS=$(subst .cpp,.o,$(SRCS))

all: imu_reader

imu_reader: $(OBJS) imu_reader.cpp
	$(CXX) $(LDFLAGS) -o imu_reader imu_reader.cpp $(OBJS) $(LDLIBS)

$(IMUlib)/MPU6050.o: $(IMUlib)/MPU6050.cpp $(IMUlib)/MPU6050.h

$(I2Clib)/I2C.o: $(I2Clib)/I2Cdev.cpp $(I2Clib)/I2Cdev.h

clean:
	rm $(OBJS)
	rm imu_reader
