#include "lock_thread_pool.h"
#include <functional>
#include <chrono>
#include <iostream>

LockThreadPool::~LockThreadPool() {
	if (RunStatus::RUNNING == run_status_) {
		std::cout << "~LockThreadPool" << std::endl;
		Destory();
	}
	else {
		std::cout << "ERROR, thread not running." << std::endl;
	}
}

void LockThreadPool::Destory() {
	{
		std::unique_lock<std::mutex> lk(mtx_);
		run_status_ = RunStatus::STOP;

		// 必须这样做以避免线程阻塞
		cond_.notify_all();
	}

	// stop every thread 
	for (auto &thread : threads_) {
		if (thread.joinable()) {
			thread.join();
		}
	}
}

void LockThreadPool::Init() {
	run_status_ = RunStatus::RUNNING;

	for (int i = 0; i < max_num_; ++i) {
		std::cout << "bind go!" << std::endl;
		threads_.emplace_back(std::thread(std::bind(&LockThreadPool::Go, this)));
		std::this_thread::sleep_for(std::chrono::milliseconds(500));
	}
}


void LockThreadPool::AppendTask(const Task& task) {
	if (RunStatus::RUNNING == run_status_) {
		std::unique_lock<std::mutex> lk(mtx_);
		task_ques_.push(task);
		// wake a thread to to the task
		cond_.notify_one();
	}
}

// every thread go like this
void LockThreadPool::Go() {
	std::cout << "thread go!, thread id :" << std::this_thread::get_id() << std::endl;


	//每个线程将竞争从队列中接任务以执行任务
	while (RunStatus::RUNNING == run_status_) {
		Task task;
		{
			std::unique_lock<std::mutex> lk(mtx_);
			if (!task_ques_.empty()) {
				task = task_ques_.front();
				task_ques_.pop();
			}

			while (RunStatus::RUNNING == run_status_ && task_ques_.empty()) {
				cond_.wait(lk);
			}
		} // end lock

		// do the task(function)
		if (task) {
			task();
		}
	}

	std::cout << "thread end!, thread id :" << std::this_thread::get_id() << std::endl;
}


void Fun1() {
	std::cout << "Fun1, thread id :" << std::this_thread::get_id() << std::endl;
}

void Fun2(int x) {
	std::cout << x << " int the Fun2, thread id :" << std::this_thread::get_id() << std::endl;
}

int main() {

	LockThreadPool thread_pool(3);
	thread_pool.Init();
	std::cout << "pool is init." << std::endl;
	//std::this_thread::sleep_for(std::chrono::milliseconds(500));

	for (int i = 0; i < 60; i++)
	{
		//thread_pool.appendTask(fun1);
		std::cout << "AppendTask" << std::endl;
		thread_pool.AppendTask(std::bind(Fun2, i));
		std::this_thread::sleep_for(std::chrono::milliseconds(500));
	}

	thread_pool.Destory();
}





