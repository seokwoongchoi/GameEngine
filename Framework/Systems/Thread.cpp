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
	threadCount = std::thread::hardware_concurrency() - 1;//��ǻ�� ��翡 �´�
	//������ ������ ���´�.
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
		thread.join();//��� ��������� �۾����� ���������� ������ �������� �ʴ´�.

	}
	threads.clear();
	threads.shrink_to_fit();
}

bool Thread::Initialize()
{
	for (uint i = 0; i < threadCount; i++)
	{
		threads.emplace_back(std::thread(&Thread::Invoke, this));//������ ������ŭ
		//�Ҵ��س��� �׾ȿ� Invoke�Լ��� �־�д�. 
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
	//	//��Ͻ��ѵ� task���� ���� ��������ɶ� ���� ���ѷ���
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
