#include "SensorListner.h"

#include <iostream>
#include <chrono>

SensorReceiver::SensorReceiver()
	: running(false), sock(INVALID_SOCKET), port(0)
{
	WSADATA wsa;
	WSAStartup(MAKEWORD(2, 2), &wsa);

	sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (sock == INVALID_SOCKET) {
		std::cerr << "Socket creation failed\n";
		return;
	}

	u_long mode = 1;
	if (ioctlsocket(sock, FIONBIO, &mode) != 0) {
		std::cerr << "Failed to set non-blocking mode. Error: " << WSAGetLastError() << "\n";
	}

	sockaddr_in serverAddr{};
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(0);
	serverAddr.sin_addr.s_addr = INADDR_ANY;

	if (bind(sock, (sockaddr*)&serverAddr, sizeof(serverAddr)) != 0) {
		std::cerr << "Bind failed: " << WSAGetLastError() << "\n";
		closesocket(sock);
		sock = INVALID_SOCKET;
		return;
	}

	sockaddr_in assignedAddr{};
	int len = sizeof(assignedAddr);
	if (getsockname(sock, (sockaddr*)&assignedAddr, &len) == 0) {
		port = ntohs(assignedAddr.sin_port);
	}
	else {
		std::cerr << "getsockname failed: " << WSAGetLastError() << "\n";
	}

	localIPv4 = getLocalIPv4();
}

SensorReceiver::~SensorReceiver() {
	stop();
	if (sock != INVALID_SOCKET) {
		closesocket(sock);
	}
	WSACleanup();
}

void SensorReceiver::start() {
	if (running) return;
	running = true;
	receiveThread = std::thread(&SensorReceiver::receiveLoop, this);
}

void SensorReceiver::stop() {
	if (!running) return;
	running = false;
	if (receiveThread.joinable()) receiveThread.join();
}

void SensorReceiver::receiveLoop() {
	char buffer[1024];

	while (running) {
		sockaddr_in client{};
		int clientLen = sizeof(client);

		int bytes = recvfrom(sock, buffer, sizeof(buffer) - 1, 0,
			(sockaddr*)&client, &clientLen);

		if (bytes == SOCKET_ERROR) {
			int err = WSAGetLastError();
			if (err == WSAEWOULDBLOCK) {
				std::this_thread::sleep_for(std::chrono::milliseconds(5));
				continue;
			}
			else {
				std::cerr << "recvfrom error: " << err << std::endl;
				std::this_thread::sleep_for(std::chrono::milliseconds(50));
				continue;
			}
		}

		if (bytes <= 0) {
			std::this_thread::sleep_for(std::chrono::milliseconds(5));
			continue;
		}

		buffer[bytes] = '\0';

		char ipStr[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &client.sin_addr, ipStr, INET_ADDRSTRLEN);

		std::string msg(buffer);
		float ax, ay, az, gx, gy, gz;

		if (sscanf_s(msg.c_str(), "ACC:%f,%f,%f;GYRO:%f,%f,%f",
			&ax, &ay, &az, &gx, &gy, &gz) == 6)
		{
			std::lock_guard<std::mutex> lock(dataMutex);
			accel[0] = ax; accel[1] = ay; accel[2] = az;
			gyro[0] = gx; gyro[1] = gy; gyro[2] = gz;
		}
	}
}


void SensorReceiver::getAccel(float& x, float& y, float& z) {
	std::lock_guard<std::mutex> lock(dataMutex);
	x = accel[0]; y = accel[1]; z = accel[2];
}

void SensorReceiver::getGyro(float& x, float& y, float& z) {
	std::lock_guard<std::mutex> lock(dataMutex);
	x = gyro[0]; y = gyro[1]; z = gyro[2];
}

std::string SensorReceiver::getLocalIPv4() {
	char hostname[256];
	if (gethostname(hostname, sizeof(hostname)) != 0)
		return "unknown";

	hostent* he = gethostbyname(hostname);
	if (!he) return "unknown";

	for (int i = 0; he->h_addr_list[i] != nullptr; i++) {
		struct in_addr addr;
		memcpy(&addr, he->h_addr_list[i], sizeof(struct in_addr));
		std::string ip = inet_ntoa(addr);

		if (ip.rfind("127.", 0) != 0)
			return ip;
	}
	return "unknown";
}