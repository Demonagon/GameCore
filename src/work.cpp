#include "work.h"

//MAIN TEST

/*

#include <iostream>
#include <sstream>
#include <chrono>
#include <array>

#define F 30
#define W 20

std::mutex m;

class PrintWork : public Work {
	protected :
	int m_name;
	int m_count;
	public :
		PrintWork() {}
		PrintWork(int name, int count) :
			Work(),
			m_name(name),
			m_count(count) {}
		PrintWork(int name, int count, CallBack call_back) :
			Work(call_back),
			m_name(name),
			m_count(count) {}

		virtual void executeWork() {
			for(int k = 0; k < m_count; k++) {
				std::this_thread::sleep_for( std::chrono::seconds(1) );
				m.lock();
				std::cout << std::this_thread::get_id() << " -> " <<
							m_name << std::endl;
				m.unlock();
			}
		}
};

int main(void) {
	MutexSleepLock l;
	int c = W;
	//EndWorkCounter ewc(&c);

	Foreman<> foreman(F);
	std::array<PrintWork, W> works;

	std::stringstream ss;

	for(int k = 0; k < W; k++)
		works[k] = PrintWork(k, 3,
				[&c, &l]{
					c--;
					if( ! c )
						l.shake();
				}
			);
	for(int k = 0; k < W; k++)
		foreman.collect(works[k]);

	l.sleepOn();

	for(int k = 0; k < W; k++)
		works[k] = PrintWork(k + W, 3);
	for(int k = 0; k < W; k++)
		foreman.collect(works[k]);

	foreman.joinAll();
	std::cout << "done !" << std::endl;
}

//*/

//WORK

//Work::Work() : m_end_trigger(NULL) {}
Work::Work() : m_call_back( []{} ) {}

//Work::Work(EndWorkTrigger * trigger) : m_end_trigger(trigger) {}
Work::Work(CallBack call_back) : m_call_back(call_back) {}

Work::~Work() {}

void
Work::run() {
	executeWork();
	//if( m_end_trigger )
	//	m_end_trigger->trigger();
	m_call_back();
}

//LAMBDA WORK

LambdaWork::LambdaWork(Function function) : m_function(function) {}
LambdaWork::LambdaWork(Function function, CallBack call_back) :
	Work(call_back), m_function(function) {}
LambdaWork::~LambdaWork() {}

void
LambdaWork::executeWork() {
	m_function();
}

//MUTEX DATA LOCK


MutexDataLock::MutexDataLock() {}

MutexDataLock::MutexDataLock(MutexDataLock const & lock) {}

MutexDataLock::~MutexDataLock() {
	unlock();
}

void
MutexDataLock::lock() {
	m_mutex.lock();
}

bool
MutexDataLock::tryLock() {
	return m_mutex.try_lock();
}

void
MutexDataLock::unlock() {
	m_mutex.unlock();
}

//MUTEX SLEEP LOCK

ConditionVariableSleepLock::ConditionVariableSleepLock() : m_thread_count(0) {}

ConditionVariableSleepLock::ConditionVariableSleepLock(ConditionVariableSleepLock const & lock) : m_thread_count(0) {}

ConditionVariableSleepLock::~ConditionVariableSleepLock() {
	shake();
}

void
ConditionVariableSleepLock::sleepOn(Predicate predicate) {
	std::unique_lock<std::mutex> lock(m_mutex);

	m_thread_count++;
	m_condition_variable.wait(lock, predicate);
	m_thread_count--;
}

void
ConditionVariableSleepLock::shake() {
	m_condition_variable.notify_all();
}

//THREAD WORKER

ThreadWorker::ThreadWorker() : m_work_flag(true) {
	m_thread = std::thread(&Worker::work, this);
}
ThreadWorker::~ThreadWorker() { terminate(); }

void ThreadWorker::addTask(Work & work) {
	if( ! m_work_flag ) return;
	Worker::addTask(work);
}

void ThreadWorker::work() {
	DataLock * lock = static_cast<DataLock *>(&m_tasks_lock);

	while( m_work_flag || (lock->lock(), ! m_tasks.empty()) ) {
		lock->unlock();
		doATask();
	}
	lock->unlock();
}

void ThreadWorker::join() {
	m_work_flag = false;
	wakeUp();
	if( m_thread.joinable() )
		m_thread.join();
}

void ThreadWorker::terminate() {
	m_work_flag = false;
	removeTasks();
	wakeUp();
	if( m_thread.joinable() )
		m_thread.join();
}

bool ThreadWorker::isTerminated() {
	return ! m_work_flag;
}

