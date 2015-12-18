CC=gcc
CXX=g++
CPPFLAGS=-g -Wall -Wpedantic -Wextra
LDFLAGS=-g
LDLIBS=-lbcm2835 -lm

I2Clib=I2Cdev
IMUlib=MPU6050
Maglib=HMC6343

IMU_SRCS=$(I2Clib)/I2Cdev.cpp $(IMUlib)/MPU6050.cpp
IMU_OBJS=$(subst .cpp,.o,$(IMU_SRCS))

MAG_SRCS=$(I2Clib)/I2Cdev.cpp $(Maglib)/HMC6343.cpp
MAG_OBJS=$(subst .cpp,.o,$(MAG_SRCS))

all: imu_reader mag_reader

imu_reader: $(IMU_OBJS) imu_reader.cpp
	$(CXX) $(LDFLAGS) -o imu_reader imu_reader.cpp $(IMU_OBJS) $(LDLIBS)

mag_reader: $(MAG_OBJS) mag_reader.cpp
	$(CXX) $(LDFLAGS) -o mag_reader mag_reader.cpp $(MAG_OBJS) $(LDLIBS)


$(IMUlib)/MPU6050.o: $(IMUlib)/MPU6050.cpp $(IMUlib)/MPU6050.h

$(I2Clib)/I2C.o: $(I2Clib)/I2Cdev.cpp $(I2Clib)/I2Cdev.h

$(Maglib)/HMC6343.o: $(Maglib)/HMC6343.cpp $(Maglib)/HMC6343.h

clean:
	rm -f $(IMU_OBJS) $(MAG_OBJS)
	rm -f imu_reader
	rm -f mag_reader
