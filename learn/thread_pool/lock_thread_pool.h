
#include <thread>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include <functional>
#include <vector>
#include <queue>

namespace RunStatus {
	static bool RUNNING = true;
	static bool STOP = false;
} // name RunStatus

class LockThreadPool {
public:
	typedef std::function<void()> Task;

	explicit LockThreadPool(int32_t max_num) :
		max_num_(max_num), run_status_(RunStatus::STOP){}
	

	virtual ~LockThreadPool();

	// init this thread poll
	void Init();

	// destory this thread pool
	void Destory();

	// append a task, need lock here
	void AppendTask(const Task& task);

private:
	// run the tasks
	void Go();

public:
	// disable copy and assign construct
	LockThreadPool(const LockThreadPool&) = delete;
	LockThreadPool& operator =(const LockThreadPool&) = delete;

private:
	int32_t max_num_;
	std::mutex  mtx_;
	std::condition_variable cond_;
	std::atomic<bool> run_status_;
	std::vector<std::thread> threads_;
	std::queue<Task> task_ques_;
};
