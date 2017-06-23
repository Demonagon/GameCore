#ifndef COLLECTOR__H
#define COLLECTOR__H

template<typename T>
class Collector {
	public :
		virtual void collect(T object) = 0;
		virtual void discard(T object) = 0;
};

template<typename T>
class ConditionalCollector : public Collector<T> {
	public :
		virtual bool doesAccept(T object) = 0;
};

#endif
