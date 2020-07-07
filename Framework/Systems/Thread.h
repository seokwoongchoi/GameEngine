#pragma once


class Task final
{
public:
	typedef std::function<void()> Process;
public:
	Task(Process&& process) //task �ȿ� process�ְ� process�ȿ� thread
	{
		this->process = std::forward<Process>(process);
	}
	~Task() = default;

	Task(const Task&) = delete;
	Task& operator=(const Task&) = delete;

	void Execute() //����
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

	void Invoke();//ȣ��

	template<typename Process>
	void AddTask(Process&& process);
	void SetStop(bool bStop) { this->bStop = bStop; }
private:
	static Thread* instance;
	std::vector<std::thread> threads;
	std::queue<Task*> tasks;
	std::mutex taskMutex;
	std::condition_variable conditionVar; //���� ���� thread�� ����� �����ϰ��ϴº���
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
	//condition_variable�� ����Ϸ��� ��¿�� ���� lock_guard�� ������ unique������Ѵ�.
	
	tasks.push(new Task(std::bind(std::forward<Process>(process))));
	//tasks.push(new Task(std::bind(std::forward<Function>(func))));
	//���� �����带 task�� �ϳ� �����Ű�� ��
	lock.unlock();

	conditionVar.notify_one(); //�ϳ� �����.
}
