#include <Windows.h>
#include<fstream>
#include<string>

#include "PlotterDevice.h"


static void handleResponse(std::string response);
static void waitForEmptyQueue();

static HANDLE _queueEmptyEvent;

int main()
{
	int queueSize = 150;

	std::ifstream inFile;
	inFile.open("C:/Users/heroo/VisualStudioProjects/EtchASketch_ImageProcessing/sortedEdgeValuesFile.txt");
	std::vector<std::pair<int, int>> fileData;

	if (!inFile)
	{
		return -1;
	}

	while (!inFile.eof())
	{
		std::string str_x, str_y;
		int x, y;
		inFile >> str_x >> str_y;
		x = atoi(str_x.c_str());
		y = atoi(str_y.c_str());

		fileData.push_back(std::make_pair(x, y));
	}

	try
	{
		_queueEmptyEvent = CreateEvent(nullptr, false, false, nullptr);
		
		PlotterDevice device(&handleResponse);
		device.Open();

		std::vector<std::pair<int, int>>::const_iterator iterator;

		for (iterator = fileData.begin(); iterator != fileData.end(); )
		{
			for (int i = 0; i < queueSize && iterator != fileData.end(); i++, iterator++)
			{
				auto command_str = "P " + std::to_string(iterator->first) + " " + std::to_string(iterator->second);
				printf("[Out] %s\r\n", command_str.c_str());
				device.SendCommand(command_str);
			}
			device.SendCommand("D");
			printf("[Out] %s\r\n", "D");
			waitForEmptyQueue();
		}

		CloseHandle(_queueEmptyEvent);
		return 0;
	}
	catch (std::exception& e)
	{
		printf("Error: %s\r\n", e.what());
		return -1;
	}
}

static void handleResponse(std::string response)
{
	// TODO Maybe we want a proper parsing framework here
	
	if (response == "E\r")
	{
		SetEvent(_queueEmptyEvent);
	}
}

static void waitForEmptyQueue()
{
	WaitForSingleObject(_queueEmptyEvent, INFINITE);
}
