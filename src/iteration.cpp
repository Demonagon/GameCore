#include "iteration.h"
#include <utility>
#include <iostream>
#include <atomic>

//MAIN TEST

/*

#include <iostream>
#include <string>
#include <vector>

typedef enum {
	A,
	B,
	C
} Position;

class DeathEffect : public TickEffect {
	public : 
		DeathEffect() {
			getMetaData().addClass<DeathEffect>();
		} 
};

class MovingObject;

class MoveForce : public TickForce {
	public :
		Position m_go_to;
		MovingObject * m_actor;
		MoveForce(Position go_to, MovingObject * actor) {
			getMetaData().addClass<MoveForce>();
			m_go_to = go_to;
			m_actor = actor;
		}
};

class MovingObject : public IterableObject {
	protected :
		Position m_go_to;
		std::string m_name;
	public :
		MovingObject(std::string name, Position go_to) {
			m_name = name;
			m_go_to = go_to;
		}

		virtual void declareTickForces(Collector<TickForce *> & force_collector) {
			std::cout << m_name << " déclare." << std::endl;
			force_collector.collect(new MoveForce(m_go_to, this));
		}

		virtual void addTickEffect(TickEffect * effect) {
			if( effect->getMetaData().isInstanceOf<DeathEffect>() )
				std::cout << "Diantre, moi, " << m_name << ", suis eu !" <<
				std::endl;
			delete effect;
		}

		virtual void applyTickEffects() {
			std::cout << m_name << " applique." << std::endl;
		}
};

class MoveForceCollector : public ForceCollector {
	protected :
		std::vector<MoveForce *> m_moving_to_a;
		std::vector<MoveForce *> m_moving_to_b;
		std::vector<MoveForce *> m_moving_to_c;
	public :
		MoveForceCollector() : 
			ForceCollector(MetaData::typeOf<MoveForce>() ) {}
		virtual ~MoveForceCollector() {}

		virtual void collect(TickForce * f) {
			MoveForce * force = dynamic_cast<MoveForce *>(f);

			switch( force->m_go_to ) {
				case A :
					m_moving_to_a.push_back(force); break;
				case B :
					m_moving_to_b.push_back(force); break;
				case C :
					m_moving_to_c.push_back(force); break;
			}
		}

		virtual void discard(TickForce * f) {}

		virtual void compute(std::list<IterableObject *> & modification_list) {
			for(std::vector<MoveForce *> const & list :
					{m_moving_to_a, m_moving_to_b, m_moving_to_c} ) {
				if( list.size() <= 1 ) continue;
				for(MoveForce * f : list)
					f->m_actor->declareTickEffect(
						new DeathEffect(),
						modification_list);
			}
		}
		virtual void clear() {
			m_moving_to_a.clear();
			m_moving_to_b.clear();
			m_moving_to_c.clear();
		}

		virtual int getWorkCount() {
			return 1;
		};
};

int main(void) {
	MovingObject alice = MovingObject("Alice", Position::A);
	MovingObject bob = MovingObject("Bob", Position::B);
	MovingObject bob_2 = MovingObject("Bob2", Position::B);
	MovingObject charlie = MovingObject("Charlie", Position::C);

	MoveForceCollector mvc;

	Foreman<> foreman(10);
	MainIterator main_iterator(foreman);
	main_iterator.collect(&mvc);

	main_iterator.collect( std::make_pair(&alice, 0) );
	main_iterator.collect( std::make_pair(&bob, 0) );
	main_iterator.collect( std::make_pair(&charlie, 0) );

	main_iterator.collect( std::make_pair(&bob, 1) );
	main_iterator.collect( std::make_pair(&bob_2, 1) );

	main_iterator.collect( std::make_pair(&alice, 2) );
	main_iterator.collect( std::make_pair(&bob, 2) );
	main_iterator.collect( std::make_pair(&bob_2, 2) );
	main_iterator.collect( std::make_pair(&charlie, 2) );

	std::cout << "Tick 1 :" << std::endl;
	main_iterator.tick();
	std::cout << "-----------------------" << std::endl;
	std::cout << "Tick 2 :" << std::endl;
	main_iterator.tick();
	std::cout << "-----------------------" << std::endl;
	std::cout << "Tick 3 :" << std::endl;
	main_iterator.tick();
	std::cout << "-----------------------" << std::endl;

	return 0;
}

//*/

//FORCE COLLECTOR

ForceCollector::ForceCollector(MetaData::Class filter) : m_filter(filter),
	m_computing_work( []{} ) {}

ForceCollector::~ForceCollector() {}

Work &
ForceCollector::computingWork(std::list<IterableObject *> & modification_list,
					 CallBack call_back) {
	m_computing_work = LambdaWork( 
		[this, &modification_list] {
			compute(modification_list);
			clear();
		},
		call_back);
	return m_computing_work;
}

bool
ForceCollector::doesAccept(TickForce * f) {
	for(MetaData::Class c : f->getMetaData().getClasses() )
		if(c == m_filter) return true;
	return false;
}

MetaData::Class &
ForceCollector::getFilter() {
	return m_filter;
}

//PHYSICAL FORCE COLLECTOR

PhysicalForceCollector::PhysicalForceCollector() :
	ForceCollector( MetaData::typeOf<TickForce>() ) {
	m_forces = std::list<TickForce *>(); 
}

PhysicalForceCollector::~PhysicalForceCollector() {
	for(TickForce * force : m_forces)
		delete force;
}

void
PhysicalForceCollector::collect(TickForce * force) {
	m_forces.push_back(force);
}

void
PhysicalForceCollector::discard(TickForce * force) {
	m_forces.remove(force);
	delete force;
}

bool
PhysicalForceCollector::doesAccept(TickForce * force) {
	return true;
}

void
PhysicalForceCollector::compute(std::list<IterableObject *> & modification_list)
{}

void
PhysicalForceCollector::clear() {
	for(TickForce * force : m_forces)
		delete force;
	m_forces.clear();
}

int
PhysicalForceCollector::getWorkCount() {
	return 0;
}

//MAIN FORCE COLLECTOR

MainForceCollector::MainForceCollector() {
	m_collectors = std::unordered_map<MetaData::Class, ForceCollector *>();
	m_physical_collector = PhysicalForceCollector();
}

MainForceCollector::~MainForceCollector() {}

void
MainForceCollector::collect(TickForce * force) {
	for(MetaData::Class c : force->getMetaData().getClasses()) {
		if( m_collectors.count(c) > 0 )
			m_collectors[c]->collect(force);
	}
}

void
MainForceCollector::discard(TickForce * force) {
	for(MetaData::Class c : force->getMetaData().getClasses())
		if( m_collectors.count(c) > 0 )
			m_collectors[c]->discard(force);
}

void
MainForceCollector::collect(ForceCollector * collector) {
	m_collectors.insert( std::make_pair(collector->getFilter(), collector) );
}

void
MainForceCollector::discard(ForceCollector * collector) {
	m_collectors.erase( collector->getFilter() );
}

int
MainForceCollector::getWorksCount() {
	int work_num = 0;
	for(std::pair<MetaData::Class, ForceCollector *> f : m_collectors)
		work_num += f.second->getWorkCount();
	return work_num;
}

void 
MainForceCollector::declareWorks(Collector<Work &> & collector,
							std::list<IterableObject *> & modification_list,
						  	CallBack call_back) {
	for(std::pair<MetaData::Class, ForceCollector *> p : m_collectors)
		collector.collect(p.second->computingWork(modification_list, call_back) );
}

/*void
MainForceCollector::computeAll() {
	for(std::pair<MetaData::Class, ForceCollector *> force_collector
		: m_collectors) {
		force_collector.second->compute();
		force_collector.second->clear();
	}
	m_physical_collector.compute();
	m_physical_collector.clear();
}*/

void
MainForceCollector::clear() {
	m_physical_collector.clear();
}

//ITERATION TABLE

IterationTable::IterationTable() :
	m_table( std::list<std::list< IterableObject * > >() )/*,
	m_front_lock(false)*/ {
	m_table.push_back( std::list<IterableObject *>() );
}

void
IterationTable::createSpace(int delay) {
	int differential = (delay + 1) - m_table.size();
	for(int k = 0; k < differential; k++)
		m_table.push_back(std::list< IterableObject *>() );
}

void
IterationTable::insert(std::pair<IterableObject *, int> execution) {
	IterableObject * object = execution.first;
	int delay = execution.second;
	int k = 0;
	for(std::list<IterableObject *> & iteration_list : m_table) {
		if(k == delay) {
			iteration_list.push_back(object);
			break;
		}
		k++;
	}
}

void
IterationTable::collect(std::pair<IterableObject *, int> execution) {
	int delay = execution.second /*+ operatingOffSet()*/;

	createSpace(delay);
	insert(execution);
}

void
IterationTable::discard(std::pair<IterableObject *, int> execution) {
	IterableObject * object = execution.first;
	int delay = execution.second;

	//if(delay == 0 and m_front_lock) return; // Trop tard pour enlever.

	if(delay >= m_table.size() ) return;

	int k = 0;
	for(std::list<IterableObject *> & iteration_list : m_table) {
		if(k == delay) {
			iteration_list.remove(object);
			break;
		}
		k++;
	}
}

/*int
IterationTable::operatingOffSet() {
	return m_front_lock ? 1 : 0;
}

void
IterationTable::lockFront(bool state) {
	m_front_lock = state;
}*/

bool
IterationTable::has_a_front() {
	return m_table.size() > 0;
}

std::list< IterableObject * > &
IterationTable::front() {
	return m_table.front();
}

void
IterationTable::pop_front() {
	m_table.pop_front();
	if( m_table.size() == 0 )
		m_table.push_back( std::list<IterableObject *>() );
}

//MAIN ITERATOR

MainIterator::MainIterator(Foreman<> & foreman, MainIndexer & indexer) :
	m_force_collector(MainForceCollector()),
	m_iteration_table(IterationTable()),
	m_foreman(foreman),
	m_indexer(indexer) {}

MainIterator::~MainIterator() {}

void
MainIterator::tick() {
	std::list<IterableObject *> modified_objects
		= std::list<IterableObject *>();

	m_indexer.switchFlowgate(true);

	declareForces();
	computeEffects(modified_objects);
	applyEffects(modified_objects);

	m_indexer.switchFlowgate(false);

	m_iteration_table.pop_front();
}

void
MainIterator::declareForces() {
	if( ! m_iteration_table.has_a_front() ) return;

	std::list<IterableObject *> & tick_batch = m_iteration_table.front();

	ConditionVariableSleepLock sleep_lock;
	MutexDataLock counter_lock;

	std::atomic<int> counter( tick_batch.size() );

	for(IterableObject * o : tick_batch)
		m_foreman.collect(
			o->forceDeclarationWork(
				m_force_collector,
				[&counter_lock, &counter, &sleep_lock] { // Call back
				//[&counter, &sleep_lock] { // Call back
					if( counter < 0 ) return;
					counter_lock.lock();
					counter--;
					if( counter == 0 )
						sleep_lock.shake();
					counter_lock.unlock();
				}
			)
		);
	sleep_lock.sleepOn([&counter] { return counter <= 0; });
}

void
MainIterator::computeEffects(std::list<IterableObject *> & modified_objects) {
	ConditionVariableSleepLock sleep_lock;
	MutexDataLock counter_lock;

	std::atomic<int> counter( m_force_collector.getWorksCount() );

	m_force_collector.declareWorks(m_foreman,
						modified_objects,
						[&counter_lock, &counter, &sleep_lock] { // Call back
							if( counter < 0 ) return;
							counter_lock.lock();
							counter--;
							if( counter == 0 )
								sleep_lock.shake();
							counter_lock.unlock();
						});

	sleep_lock.sleepOn([&counter] { return counter <= 0; });
}

void
MainIterator::applyEffects(std::list<IterableObject *> & modified_objects) {
	ConditionVariableSleepLock sleep_lock;
	MutexDataLock counter_lock;

	std::atomic<int> counter( modified_objects.size() );

	for(IterableObject * o : modified_objects)
		m_foreman.collect(
			o->effectsApplicationWork(
				[&counter_lock, &counter, &sleep_lock] { // Call back
					if( counter < 0 ) return;
					counter_lock.lock();
					counter--;
					if( counter == 0 )
						sleep_lock.shake();
					counter_lock.unlock();
				}
			)
		);

	sleep_lock.sleepOn([&counter] { return counter <= 0; });
}

void
MainIterator::collect(ForceCollector * collector) {
	m_force_collector.collect(collector);
}
void
MainIterator::discard(ForceCollector * collector) {
	m_force_collector.discard(collector);
}

void
MainIterator::collect(std::pair<IterableObject *, int> execution) {
	m_iteration_table.collect(execution);
}
void
MainIterator::discard(std::pair<IterableObject *, int> execution) {
	m_iteration_table.discard(execution);
}
