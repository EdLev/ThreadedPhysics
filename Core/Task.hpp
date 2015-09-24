#pragma once

#include <vector>
#include <atomic>

//a pool of threads dedicated to running a particular function on input and output buffers of data
template <class InputDataContainerType, class OutputDataContainerType, class FunctionType, class ExtraObjectType>
class Task
{
public:
	//double-pointers so the application can manage which buffers we read/write
	Task(unsigned int NumThreads, InputDataContainerType** InInputBuffer, OutputDataContainerType** InOutputBuffer, ExtraObjectType* ExtraObject)
		:	InputBuffer(InInputBuffer),
			OutputBuffer(InOutputBuffer),
			bShutdown(false),
			bFinished(false),
			bDoWork(false),
			CurrentDataIndex(0)
	{
		for (size_t threadIndex = 0; threadIndex < NumThreads; ++threadIndex)
		{
			BackgroundThreads.push_back(std::thread(&Task::ThreadWork, this, ExtraObject));
		}

		for (auto& thread : BackgroundThreads)
		{
			thread.detach();
		}
	}

	~Task()
	{
		bShutdown = true;

		//wait for threads to finish to prevent them from accessing deallocated memory after we're destroyed
		while (NumShutdownThreads < BackgroundThreads.size())
		{
			std::this_thread::yield();
		}
	}

	//manage flags to make threads iterate through data buffer and do work, returns when finished
	void Work()
	{
		CurrentDataIndex = 0;
		bFinished = false;
		bDoWork = true;
		while (!bFinished)
		{
			std::this_thread::yield();
		}
		bDoWork = false;
	}

private:

	void ThreadWork(ExtraObjectType* ExtraObject)
	{
		while (!bShutdown)
		{
			if (bDoWork)
			{
				size_t currentIndex;
				while ((currentIndex = CurrentDataIndex++) < (**InputBuffer).size())
				{
					InnerFunction(InputBuffer, OutputBuffer, currentIndex, ExtraObject);	

					if (currentIndex == (**InputBuffer).size() - 1)
					{
						bFinished = true;
						break;
					}
				}
				
				if ((**InputBuffer).empty())
				{
					bFinished = true;
				}

				//yield if we're out of the loop, to avoid spending time aggressively checking array bounds if we've finished
				std::this_thread::yield();
			}
			else
			{
				//yield to other threads while we wait for our turn
				std::this_thread::yield();
			}
		}

		//signal the main thread that this thread is done
		++NumShutdownThreads;
	}

	std::vector<std::thread> BackgroundThreads;

	std::atomic<size_t> CurrentDataIndex;
	std::atomic<bool> bShutdown;
	std::atomic<bool> bDoWork;
	std::atomic<bool> bFinished;
	std::atomic<unsigned int> NumShutdownThreads;

	FunctionType InnerFunction;

	InputDataContainerType** InputBuffer;
	OutputDataContainerType** OutputBuffer;
};