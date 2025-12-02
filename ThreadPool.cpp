#include "ThreadPool.hpp"
#include "Threads.hpp"
#include <cstdio>

ThreadPool *ThreadPoolNew(Arena* arena, u32 threadsCount)
{
	ThreadPool* threadPool = PushArray(arena, ThreadPool, 1);

	DllInit(threadPool->taskQueueFreeSpots);

	InitializeCriticalSection(&threadPool->cs);
	InitializeCriticalSection(&threadPool->taskFreeSpotsCS);
	InitializeConditionVariable(&threadPool->cv);

	OsHandle* workerThreads = PushArray(arena, OsHandle, threadsCount);
	
	ThreadStartParams* threadStartParams = PushArray(arena, ThreadStartParams, 1);
	threadStartParams->routine = ThreadPoolWorkerWrapper;
	threadStartParams->param = threadPool;

	for (size_t i = 0; i < threadsCount; i++)
	{
		workerThreads[i] = OsCreateThread(ThreadMainEntry, threadStartParams);
	}

	threadPool->workerThreads = workerThreads;
	threadPool->arena = arena;
	threadPool->workerThreadCount = threadsCount;
	return threadPool;
}

void ThreadPoolSubmit(ThreadPool* threadPool, TaskFunction taskFunction, void* taskParam)
{
	EnterCriticalSection(&threadPool->cs);
	{
		if (!DllEmpty(threadPool->taskQueueFreeSpots))
		{
			EnterCriticalSection(&threadPool->taskFreeSpotsCS);
			{
				Task* task = DllPop(threadPool->taskQueueFreeSpots, taskLinks);

				task->worker = taskFunction;
				task->param = taskParam;
				QueuePush(threadPool->taskQueue, taskQueueLinks, task);
			}
			LeaveCriticalSection(&threadPool->taskFreeSpotsCS);
		}

		else
		{
			Task* task = TaskNew(threadPool->arena, taskFunction, taskParam);
			QueuePush(threadPool->taskQueue, taskQueueLinks, task);
		}

		WakeConditionVariable(&threadPool->cv);
	}
	LeaveCriticalSection(&threadPool->cs);
}

void ThreadPoolSubmit(ThreadPool* threadPool, TaskFunction taskFunction, void* taskParam, size_t taskParamSize)
{
	EnterCriticalSection(&threadPool->cs);
	{
		if (!DllEmpty(threadPool->taskQueueFreeSpots))
		{
			EnterCriticalSection(&threadPool->taskFreeSpotsCS);
			{
#if 1
				Task* task = DllPop(threadPool->taskQueueFreeSpots, taskLinks);
				task->worker = taskFunction;
				task->paramSize = taskParamSize;

				// Assuming we are pushing tasks of same worker, same param.
				// Which is most of the cases will be like that.
				memcpy(task->paramData, taskParam, taskParamSize);
				QueuePush(threadPool->taskQueue, taskQueueLinks, task);
#endif
			}
			LeaveCriticalSection(&threadPool->taskFreeSpotsCS);
		}

		else
		{
			Task* task = TaskNew(threadPool->arena, taskFunction, taskParam, taskParamSize);
			QueuePush(threadPool->taskQueue, taskQueueLinks, task);
		}

		WakeConditionVariable(&threadPool->cv);
	}
	LeaveCriticalSection(&threadPool->cs);
}

Task* TaskNew(Arena* arena, TaskFunction taskFunction, void* taskParam)
{
	Task* task = PushArray(arena, Task, 1);
	task->worker = taskFunction;
	task->param = taskParam;
	return task;
}

Task* TaskNew(Arena* arena, TaskFunction taskFunction, void* taskParam, size_t taskParamSize)
{
	Task* task = PushVarSzStruct(arena, Task, sizeof(Task) + taskParamSize);
	task->worker = taskFunction;
	task->paramSize = taskParamSize;
	memcpy(task->paramData, taskParam, taskParamSize);
	return task;
}

u32 ThreadPoolWorkerWrapper(void* param)
{
	ThreadPool* threadPool = (ThreadPool*)param;

	for (;;)
	{
		Task* task{};

		EnterCriticalSection(&threadPool->cs);
		{
			while (QueueEmpty(threadPool->taskQueue))
			{
				SleepConditionVariableCS(&threadPool->cv, &threadPool->cs, INFINITE);
			}

			task = QueuePop(threadPool->taskQueue, taskQueueLinks);
		}
		LeaveCriticalSection(&threadPool->cs);

		if (task)
		{
			bool persist = false;
			task->worker(task->paramData, &persist);
			
			if (persist)
			{
				EnterCriticalSection(&threadPool->cs);
				{
					QueuePush(threadPool->taskQueue, taskQueueLinks, task);
				}
				LeaveCriticalSection(&threadPool->cs);
			}

			else
			{
				EnterCriticalSection(&threadPool->taskFreeSpotsCS);
				{
					DllPush(threadPool->taskQueueFreeSpots, taskLinks, task);
				}
				LeaveCriticalSection(&threadPool->taskFreeSpotsCS);
			}
		}
	}
}

void ThreadPoolWaitForAll(ThreadPool* threadPool)
{
	WaitForMultipleObjects(threadPool->workerThreadCount, (HANDLE*)threadPool->workerThreads, true, INFINITE);
}
