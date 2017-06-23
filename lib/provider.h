#ifndef PROVIDER__H
#define PROVIDER__H
#include "collector.h"
#include <list>

template<typename T>
class Provider {
	public :
		virtual void addCollector(Collector<T> & collector) = 0;
		virtual void removeCollector(Collector<T> & collector) = 0;
};

template<typename T>
class StandardListProvider : public Provider<T> {
	protected :
		std::list<Collector<T> *> m_collectors;
	public :
		void provide(T event) {
			for(Collector<T> * collector : m_collectors)
				collector->collect(event);
		}

		void addCollector(Collector<T> & collector) {
			m_collectors.push_back(& collector);
		}

		void removeCollector(Collector<T> & collector) {
			m_collectors.remove(& collector);
		}
};

#endif
