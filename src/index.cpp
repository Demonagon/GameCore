#include "index.h"
#include <utility>

//MAIN TEST
/*

#include <iostream>

class A : public virtual MetaDataObject {
	public :
		A() { getMetaData().addClass<A>(); }
		virtual ~A() { std::cout << "Aurevoir !" << std::endl; }
};

class B : public virtual MetaDataObject {
	public :
		B() { getMetaData().addClass<B>(); }
		virtual ~B() { std::cout << "Burevoir !" << std::endl; }
};

class C : public A, B {
	public :
		C() { getMetaData().addClass<C>(); }
		virtual ~C() { std::cout << "Curevoir !" << std::endl; }
};

class ListenerA : public Collector<MetaDataObject *> {
	public :
		virtual void collect(MetaDataObject * object) {
			std::cout << "A !" << std::endl;
		}

		virtual void discard(MetaDataObject * object) {
			std::cout << "A..." << std::endl;
		}
};

class ListenerB : public Collector<MetaDataObject *> {
	public :
		virtual void collect(MetaDataObject * object) {
			std::cout << "B !" << std::endl;
		}

		virtual void discard(MetaDataObject * object) {
			std::cout << "B..." << std::endl;
		}
};

class ListenerC : public Collector<MetaDataObject *> {
	public :
		virtual void collect(MetaDataObject * object) {
			std::cout << "C !" << std::endl;
		}

		virtual void discard(MetaDataObject * object) {
			std::cout << "C..." << std::endl;
		}
};

int main(void) {
	ListenerA la = ListenerA();
	ListenerB lb = ListenerB();
	ListenerC lc = ListenerC();

	MainIndexer main_index = MainIndexer();
	main_index.collect( new Indexer(MetaData::typeOf<A>()) );
	main_index.collect( new Indexer(MetaData::typeOf<B>()) );
	main_index.collect( new Indexer(MetaData::typeOf<C>()) );

	main_index.getIndexer( MetaData::typeOf<A>() )->addCollector(la);
	main_index.getIndexer( MetaData::typeOf<B>() )->addCollector(lb);
	main_index.getIndexer( MetaData::typeOf<C>() )->addCollector(lc);

	std::cout << "---|---" << std::endl;
	main_index.collect( new A() );
	std::cout << "---|---" << std::endl;
	main_index.collect( new B() );
	std::cout << "---|---" << std::endl;
	main_index.collect( new C() );
	std::cout << "---|---" << std::endl;

	return 0;
}

//*/
//INDEXER

Indexer::Indexer(MetaData::Class filter) :
	m_filter(filter),
	//m_flowgate_activated(false),
	//m_enter_lock(),
	//m_enter_list(),
	//m_exit_lock(),
	//m_exit_list(),
	m_objects_lock(),
	m_objects(std::unordered_map<MetaDataObject *, MetaDataObject *>()),
	m_listeners(std::list<Collector<MetaDataObject *> * >())
{}

Indexer::~Indexer() {}

MetaData::Class Indexer::getFilter() {
	return m_filter;
}

std::unordered_map<MetaDataObject *, MetaDataObject *> &
Indexer::getObjects() {
	return m_objects;
}

bool Indexer::doesAccept(MetaDataObject * object) {
	return object->getMetaData().isInstanceOf(m_filter);
}

void Indexer::collect(MetaDataObject * object) {
	//if( ! m_flowgate_activated ) {
		m_objects_lock.lock();
			m_objects.insert(std::make_pair(object, object) );
		m_objects_lock.unlock();
		for( Collector<MetaDataObject *> * listener : m_listeners )
			listener->collect(object);
	/*}
	else {
		m_enter_lock.lock();
			m_enter_list.push_back(object);
		m_enter_lock.unlock();
	}*/
}

void Indexer::discard(MetaDataObject * object) {
	//if( ! m_flowgate_activated ) {
		for( Collector<MetaDataObject *> * listener : m_listeners )
			listener->collect(object);
		m_objects_lock.lock();
			m_objects.erase(object);
		m_objects_lock.unlock();
	//}
	/*else {
		m_exist_lock.lock();
			m_exit_list.push_back(object);
		m_exit_lock.unlock();
	}*/
}

/*void Indexer::switchFlowgate(bool activated) {
	bool old_activation = m_flowgate_activated;
	if( ! old_activation ) {
		m_flowgate_activated = activated;
		return;
	}
	if( activated ) return;

	m_flowgate_activated = false;

	m_exit_lock.lock();
		for(MetaDataObject * object : m_exit_list)
			discard(object);
		m_exit_list.clear();
	m_exit_lock.unlock();

	m_enter_lock.lock();
		for(MetaDataObject * object : m_enter_list)
			collect(object);
		m_enter_list.clear();
	m_enter_lock.unlock();
}*/

void Indexer::addCollector(Collector<MetaDataObject *> & collector) {
	m_listeners.push_back(&collector);
}

void Indexer::removeCollector(Collector<MetaDataObject *> & collector) {
	m_listeners.remove(&collector);
}

//PHYSICAL INDEXER

PhysicalIndexer::PhysicalIndexer() :
	Indexer(MetaData::typeOf<MetaDataObject>()),
	m_flowgate_activated(false)
{}

PhysicalIndexer::~PhysicalIndexer() {
	for(std::pair<MetaDataObject *, MetaDataObject *> object : m_objects)
		delete object.first;
}

void
PhysicalIndexer::collect(MetaDataObject * object) {
	if( ! m_flowgate_activated )
		Indexer::collect(object);
	else {
		m_enter_lock.lock();
			m_enter_list.push_back(object);
		m_enter_lock.unlock();
	}
}

void
PhysicalIndexer::discard(MetaDataObject * object) {
	if( ! m_flowgate_activated ) {
		Indexer::discard(object);
		delete object;
	}
	else {
		m_exit_lock.lock();
			m_exit_list.push_back(object);
		m_exit_lock.unlock();
	}
}

void
PhysicalIndexer::switchFlowgate(bool activated) {
	bool old_activation = m_flowgate_activated;
	if( ! old_activation ) {
		m_flowgate_activated = activated;
		return;
	}
	if( activated ) return;

	m_flowgate_activated = false;

	m_exit_lock.lock();
		for(MetaDataObject * object : m_exit_list)
			discard(object);
		m_exit_list.clear();
	m_exit_lock.unlock();

	m_enter_lock.lock();
		for(MetaDataObject * object : m_enter_list)
			collect(object);
		m_enter_list.clear();
	m_enter_lock.unlock();
}

//MAIN INDEXER

MainIndexer::MainIndexer() :
	m_indexers(std::unordered_map<MetaData::Class, Indexer *>()) {}

MainIndexer::~MainIndexer() {
	for(std::pair<MetaData::Class, Indexer *> indexer : m_indexers)
		delete indexer.second;
}

void MainIndexer::collect(MetaDataObject * object) {
	m_physical_indexer.collect(object);
	for(MetaData::Class c : object->getMetaData().getClasses())
		if( m_indexers.count(c) != 0 )
			getIndexer(c)->collect(object);
}

void MainIndexer::discard(MetaDataObject * object) {
	for(MetaData::Class c : object->getMetaData().getClasses())
		if( m_indexers.count(c) != 0 )
			getIndexer(c)->discard(object);
	m_physical_indexer.discard(object);
}

void MainIndexer::collect(Indexer * object) {
	m_indexers.insert(std::make_pair(object->getFilter(), object));
}

void MainIndexer::discard(Indexer * object) {
	m_indexers.erase(object->getFilter());
	delete object;
}

void MainIndexer::switchFlowgate(bool activated) {
	m_physical_indexer.switchFlowgate(activated);
}

Indexer * MainIndexer::getIndexer(MetaData::Class filter) {
	return m_indexers[filter];
}

