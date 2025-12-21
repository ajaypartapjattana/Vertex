#pragma once

#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <WinSock2.h>
#include <WS2tcpip.h>
#include <iphlpapi.h>

#include <thread>
#include <atomic>
#include <mutex>
#include <string>

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "Iphlpapi.lib")

class SensorReceiver {
public:
	SensorReceiver();
	~SensorReceiver();

	void start();
	void stop();

	void getAccel(float& x, float& y, float& z);
	void getGyro(float& x, float& y, float& z);

	int getPort() const { return port; }
	std::string getLocalIPv4();

	std::string localIPv4 = "___.___.___.___";

private:
	void receiveLoop();

	int port;
	SOCKET sock;

	std::thread receiveThread;
	std::atomic<bool> running;

	float accel[3] = { 0,0,0 };
	float gyro[3] = { 0,0,0 };
	std::mutex dataMutex;
};