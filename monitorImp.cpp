#include "monitor.h"
// Constructor & Deconstructor

monitor::monitor()
{
	initializedTime = clock();
	initialized = false;
	szDataSource, szMachineName = NULL;
	cpuCounterPath = (LPCSTR)"\\Processor(_Total)\\% Processor Time";	//the Windows 10 PerfMon counter location
	PdhOpenQuery(nullptr, 0, &queryHandle);				//Initialize our new query to realtime data
	PdhAddCounterA(queryHandle, cpuCounterPath, 0, &cpuCounter);	//set the address of our new counter to cpuCounter
	PdhCollectQueryData(queryHandle);;
	updateScheme();
	initialized = true;
}; // end monitor()

monitor::~monitor()
{
	cpuData.clear();
	if (queryHandle) PdhCloseQuery(queryHandle);
	std::cout << "The program ran for " << (clock() - initializedTime) << " seconds." << std::endl;
}; // end ~monitor()

// Accessor Definitions

double monitor::viewCpuLoad()
{
	return cpuLoadValue.doubleValue; //PDH_FMT_COUNTERVALUE
}; // end viewCpuLoad()

std::string monitor::viewCurrentScheme()
{
	return currentScheme;
}; // end viewCurrentScheme()

clock_t monitor::viewStartTime()
{
	return startTime;
}; //  end viewStartTime()

// Mutator Denitions

void monitor::collectCounterData()
{
	PdhCollectQueryData(queryHandle);
	updateCpuLoad();
	updateDataArrays();
	return;
};

void monitor::updateCpuLoad()
{
	PdhGetFormattedCounterValue(cpuCounter, PDH_FMT_DOUBLE, NULL, &cpuLoadValue); // PDH_FMT_COUNTERVALUE
	return;
}; // end updateCpuLoad()

void monitor::updateDataArrays()
{
	if (arraySize == 0)
	{
		cpuData.insert(cpuData.begin(), viewCpuLoad());
	}
	else if ((arraySize < 60) && (arraySize > 0))
	{
		for (int i = 0; i <= arraySize; i++)
		{
			if (i == arraySize)
				cpuData.insert(cpuData.begin(), viewCpuLoad());
		};
	}
	else if (arraySize == 60)
	{
		cpuData.pop_back();
		cpuData.insert(cpuData.begin(), viewCpuLoad());
		arraySize--;
	};
	arraySize++;
	return;
}; // end updateDataArrays

void monitor::updateScheme()
{
	system("powercfg getactivescheme > currentScheme.txt");
	std::ifstream inData;
	inData.open("currentScheme.txt");
	getline(inData, currentScheme);
	inData.close();
	return;
}; // end updateScheme()

void monitor::setStartTime()
{
	startTime = clock();
	return;
}; // end setStartTime()

void monitor::setCurrentScheme(bool scheme)
{
	if (scheme)
	{
		system("powercfg s scheme_min");	// High Performance Mode
	}
	else
	{
		system("powercfg s scheme_max");	// Power Saving Mode
	}
	return;
}; // end setCurrentScheme()

void monitor::primaryRoutine()
{
	std::cout << "Monitoring";
	setStartTime();
	if (initialized == true)
	{
		startTime = startTime + initializedTime;
		initialized = false;
	}

	while ((clock() - viewStartTime()) <= 60000)	//60000miliseconds for one minute
	{
		std::cout << ".";
		Sleep(1000);
	}
	collectCounterData();
	std::cout << "\n" <<"The current average CPU Load over the last minutes is " << cpuData.front() << std::endl;

	totalCpuData = 0;
	fiveCpuData = 0;
	if (cpuData.size() >= 5)
	{
		for (int i = 0; i < 5; i++)
			fiveCpuData = fiveCpuData + cpuData[i];
		fiveCpuData = fiveCpuData / 5;
		std::cout << "The current average CPU Load over the five minutes is " << fiveCpuData << std::endl;
	
		for (int i = 0; i < cpuData.size(); i++)
			totalCpuData = totalCpuData + cpuData[i];

		totalCpuData = totalCpuData / (cpuData.size());
		std::cout << "The current average CPU Load over the last hour is " << totalCpuData << std::endl;
	
		if ((fiveCpuData > 33) && (fiveCpuData > totalCpuData))
		{
			std::cout << "Setting to High Performance Mode" << std::endl;
			setCurrentScheme(true);
			updateScheme();
			std::cout << "The current " << viewCurrentScheme() << std::endl;
			return;
		}
		else if ((fiveCpuData < 25) && (fiveCpuData <= totalCpuData))
		{
			std::cout << "Setting to Power Saver Mode" << std::endl;
			setCurrentScheme(false);
			updateScheme();
			std::cout << "The current " << viewCurrentScheme() << std::endl;
			return;
		}
		else
			return;
	}
}; // end controlSequence()