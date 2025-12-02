#pragma once
#include <Windows.h>
#include "Arena.hpp"
#include "Types.hpp"
#include "DataStructures.hpp"
#include "OS.hpp"

using TaskFunction = void(*)(void*, bool*);

struct Task
{
	DllEntry(Task) taskLinks;
	QueueEntry(Task) taskQueueLinks;

	TaskFunction worker;
	size_t paramSize;
	void* param;

	char paramData[1];
};

struct ThreadPool
{
	Arena* arena;
	OsHandle* workerThreads;
	u32 workerThreadCount;
	CRITICAL_SECTION cs;
	CRITICAL_SECTION taskFreeSpotsCS;
	CONDITION_VARIABLE cv;
	
	Dll(Task) taskQueueFreeSpots;
	Queue(Task) taskQueue;
};

ThreadPool* ThreadPoolNew(Arena* arena, u32 threadsCount);
void ThreadPoolSubmit(ThreadPool* threadPool, TaskFunction taskFunction, void* taskParam);
void ThreadPoolSubmit(ThreadPool* threadPool, TaskFunction taskFunction, void* taskParam, size_t taskParamSize);
Task* TaskNew(Arena* arena, TaskFunction taskFunction, void* taskParam);
Task* TaskNew(Arena* arena, TaskFunction taskFunction, void* taskParam, size_t taskParamSize);
u32 ThreadPoolWorkerWrapper(void* param);
void ThreadPoolWaitForAll(ThreadPool* threadPool);
