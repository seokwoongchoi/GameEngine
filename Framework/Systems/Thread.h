#pragma once


class Task final
{
public:
	typedef std::function<void()> Process;
public:
	Task(Process&& process) //task 안에 process있고 process안에 thread
	{
		this->process = std::forward<Process>(process);
	}
	~Task() = default;

	Task(const Task&) = delete;
	Task& operator=(const Task&) = delete;

	void Execute() //실행
	{
		process();
	}
private:
	Process process;
};

class Thread final 
{
public:
	Thread();
	~Thread();

	static void Create();
	static void Delete();

	static Thread* Get();

	Thread(const Thread&) = delete;
	Thread& operator=(const Thread&) = delete;

	bool Initialize();

	void Invoke();//호출

	template<typename Process>
	void AddTask(Process&& process);
	void SetStop(bool bStop) { this->bStop = bStop; }
private:
	static Thread* instance;
	std::vector<std::thread> threads;
	std::queue<Task*> tasks;
	std::mutex taskMutex;
	std::condition_variable conditionVar; //조건 변수 thread간 통신을 가능하게하는변수
	uint threadCount;
	bool bStop;

};

template<typename Process>
inline void Thread::AddTask(Process && process)
{
	if (threads.empty())
	{
		//LOG_WARNING("THREAD::AddTask:: No available threads");
		process();
		return;
	}
	
	std::unique_lock<std::mutex> lock(taskMutex);
	//condition_variable를 사용하려면 어쩔수 없이 lock_guard를 못쓰고 unique를써야한다.
	
	tasks.push(new Task(std::bind(std::forward<Process>(process))));
	//tasks.push(new Task(std::bind(std::forward<Function>(func))));
	//들어온 쓰레드를 task에 하나 저장시키고 락
	lock.unlock();

	conditionVar.notify_one(); //하나 끼운다.
}
