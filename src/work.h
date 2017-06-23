#ifndef WORK__H
#define WORK__H
#include <list>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <functional>
#include <iostream>
#include "collector.h"

/**
* Cette classe permet d'effectuer des calculs à la fin d'un travail, même si
* plusieurs travaux sont gérés par un même thread.
*/
/**class EndWorkTrigger {
	public :
		virtual void trigger() = 0;
};**/

typedef std::function<void()> Function;
typedef std::function<void()> CallBack;
typedef std::function<bool()> Predicate;

/**
* Représente une tâche à exécuter. Possède un trigger de fin d'exécution.
*/
class Work {
	protected :
		//EndWorkTrigger * m_end_trigger;
		CallBack m_call_back;
	public :
		Work();
		//Work(EndWorkTrigger * trigger);
		Work(CallBack call_back);
		virtual ~Work();

		virtual void run();

		virtual void executeWork() = 0;
};

class LambdaWork : public Work {
	protected :
		Function m_function;
	public :
		LambdaWork(Function function);
		LambdaWork(Function function, CallBack call_back);
		virtual ~LambdaWork();

		virtual void executeWork();
};

/**
* Représente un verrou classique.
*/

class DataLock {
	public :
		virtual ~DataLock() {}

		virtual void lock() = 0;
		virtual bool tryLock() = 0;
		virtual void unlock() = 0;
};

class MutexDataLock : public DataLock {
	protected :
		std::mutex m_mutex;
	public :
		MutexDataLock();
		MutexDataLock(MutexDataLock const & lock);
		virtual ~MutexDataLock();

		virtual void lock();
		virtual bool tryLock();
		virtual void unlock();
};

/**
* Représente un verrou sur lequel des travailleurs peuvent se reposer. Remuer
* le verrou fait se réveiller les travailleurs, qui se remettent au travail.
*/
class SleepLock {
	public :

		virtual void sleepOn(Predicate predicate) = 0;
		virtual void shake() =  0;
};

class ConditionVariableSleepLock : public SleepLock {
	protected :
		std::mutex m_mutex;
		std::condition_variable m_condition_variable;
		std::atomic<int> m_thread_count;
	public :
		ConditionVariableSleepLock();
		ConditionVariableSleepLock(ConditionVariableSleepLock const & lock);
		virtual ~ConditionVariableSleepLock();

		virtual void sleepOn(Predicate predicate);
		virtual void shake();
};

/**
* Le travailleur possède une liste de tâches qu'il doit exécuter.
* Lorsque cette liste est vide, le travailleur ne meurt pas, mais attend d'être
* assigné à une nouvelle tâche.
*/

template <class DataLockType = MutexDataLock,
		  class SleepLockType = ConditionVariableSleepLock>
class Worker {
	protected :
		DataLockType m_tasks_lock;
		SleepLockType m_sleep_lock;

		std::list<Work *> m_tasks;

		virtual void sleep(Predicate predicate) {
			static_cast<SleepLock *>(&m_sleep_lock)->sleepOn(predicate);
		}

	public :
		Worker() :  m_tasks_lock(DataLockType()),
					m_sleep_lock(SleepLockType()),
					m_tasks(std::list<Work *>())
			{}
		virtual ~Worker() {}

		virtual void addTask(Work & work) {
			DataLock * lock = static_cast<DataLock *>(&m_tasks_lock);

			lock->lock();
			m_tasks.push_back(&work);
			lock->unlock();
			wakeUp();
		}

		void removeTasks() {
			DataLock * lock = static_cast<DataLock *>(&m_tasks_lock);
			
			lock->lock();
			m_tasks.clear();
			lock->unlock();
		}

		void wakeUp() {
			static_cast<SleepLock *>(&m_sleep_lock)->shake();
		}

		void doATask() {
			DataLock * lock = static_cast<DataLock *>(&m_tasks_lock);

			if( lock->lock(), m_tasks.empty() ) {
				lock->unlock();
				sleep([this] { return ! m_tasks.empty() || isTerminated(); } );
			}
			else {
				Work * current_work = m_tasks.front();
				m_tasks.pop_front();
				lock->unlock();
				current_work->run();
			}
		}

		virtual void work() = 0;
		virtual void join() = 0;
		virtual void terminate() = 0;
		virtual bool isTerminated() = 0;
};

class ThreadWorker : public Worker<> {
	protected :
		std::atomic<bool> m_work_flag;
		std::thread m_thread;
	public :
		ThreadWorker();
		virtual ~ThreadWorker();

		virtual void addTask(Work & work);

		virtual void work();
		virtual void join();
		virtual void terminate();
		virtual bool isTerminated();
};

/**
* Contremaître qui assigne le travail aux travailleurs. Les tâches sont
* distribuées immédiatement.
*/
template <class WorkerType = ThreadWorker>
class Foreman : public Collector<Work &> {
	protected :
		int m_current_index;
		std::vector<Worker<> *> m_workers;
	public :
		Foreman(int worker_count) :
			m_current_index(0),
			m_workers(std::vector<Worker<> *>(worker_count)) {
			for(int k = 0; k < worker_count; k++)
				m_workers[k] = new WorkerType();
		}

		virtual ~Foreman() {
			joinAll();
			for(Worker<> * w : m_workers)
				delete w;
		}

		virtual void collect(Work & work) {
			m_workers[m_current_index]->addTask(work);
			m_current_index = (m_current_index + 1) % m_workers.size();
		}

		virtual void discard(Work & work) {}

		virtual void joinAll() {
			for(Worker<> * w : m_workers)
				w->join();
		}

		virtual void terminateAll() {
			for(Worker<> * w : m_workers)
				w->terminate();
		}
};

#endif
