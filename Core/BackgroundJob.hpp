#include <vector>
#include <atomic>
#include <thread>
#include <mutex>

//a pool of threads dedicated to running a particular function on a buffer of data
template <class InputDataContainerType, class OutputDataContainerType, class FunctionType, class ExtraObjectType>
class BackgroundJob
{
public:
	BackgroundJob(unsigned int NumThreads, InputDataContainerType** InInputBuffer, OutputDataContainerType** InOutputBuffer, FunctionType InInnerFunction, ExtraObjectType* ExtraObject)
		:	InputBuffer( InInputBuffer ),
			OutputBuffer( InOutputBuffer ),
			InnerFunction( InInnerFunction ),
			bShutdown( false ),
			bFinished( false ),
			bDoWork( false ),
			CurrentDataIndex( 0 )
	{
		for (size_t threadIndex = 0; threadIndex < NumThreads; ++threadIndex )
		{
			BackgroundThreads.push_back(std::thread(&BackgroundJob::ThreadWork, this, ExtraObject));
		}

		for (auto& thread : BackgroundThreads)
		{
			thread.detach();
		}
	}

	~BackgroundJob()
	{
		bShutdown = true;

		//wait for threads to return
		while (NumFinishedThreads < BackgroundThreads.size())
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

	void ThreadWork( ExtraObjectType* ExtraObject )
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
				std::this_thread::yield();
			}
		}

		++NumFinishedThreads;
	}

	std::vector<std::thread> BackgroundThreads;

	std::atomic<size_t> CurrentDataIndex;
	std::atomic<bool> bShutdown;
	std::atomic<bool> bDoWork;
	std::atomic<bool> bFinished;
	std::atomic<unsigned int> NumFinishedThreads;

	FunctionType* InnerFunction;

	InputDataContainerType** InputBuffer;
	OutputDataContainerType** OutputBuffer;
};