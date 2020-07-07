#include "Framework.h"
#include "Thread.h"

Thread* Thread::instance = NULL;

void Thread::Create()
{
	assert(instance == NULL);

	instance = new Thread();
	//instance->Initialize();
}

void Thread::Delete()
{
	SafeDelete(instance);
}

Thread * Thread::Get()
{
	return instance;
}
Thread::Thread()
	:bStop(false)
{
	threadCount = std::thread::hardware_concurrency() - 1;//컴퓨터 사양에 맞는
	//쓰레드 갯수를 얻어온다.
	//cout << to_string(threadCount) << endl;
	Initialize();
}

Thread::~Thread()
{
	std::unique_lock<std::mutex> lock(taskMutex);

	bStop = true;

	lock.unlock();

	conditionVar.notify_all();
	for (auto& thread : threads)
	{
		thread.join();//모든 쓰레드들의 작업들이 끝나기전엔 밑으로 내려가지 않는다.

	}
	threads.clear();
	threads.shrink_to_fit();
}

bool Thread::Initialize()
{
	for (uint i = 0; i < threadCount; i++)
	{
		threads.emplace_back(std::thread(&Thread::Invoke, this));//쓰레드 갯수만큼
		//할당해놓고 그안에 Invoke함수를 넣어둔다. 
	}
	return true;
}

void Thread::Invoke()
{
	//std::shared_ptr<Task> task;
	//
	//while (true)
	//{
	//	std::unique_lock<std::mutex> lock(taskMutex);

	//	conditionVar.wait(lock, [this]() {return !tasks.empty() || bStop; });

	//	if (bStop &&tasks.empty())
	//		return;

	//	task = tasks.front();
	//	tasks.pop();

	//	lock.unlock();

	//	task->Execute();
	//	
	//	//등록시켜둔 task들이 전부 실행종료될때 까지 무한룰프
	//}
	Task* task = nullptr;

	while (true)
	{
		std::unique_lock<std::mutex> lock(taskMutex);

		conditionVar.wait(lock, [this]() { return !tasks.empty() || bStop; });

		if (bStop && tasks.empty())
			return;

		task = tasks.front();

		tasks.pop();

		lock.unlock();

		task->Execute();

		SafeDelete(task);
	}
	
}
