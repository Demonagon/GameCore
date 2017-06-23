#include "test.h"
#include "metadata.h"
#include "collector.h"
#include "work.h"
#include "iterable_object.h"
#include "iteration.h"
#include "index.h"
#include <utility>
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
		MainIterator & m_mit;
		MainIndexer & m_min;
		bool dead;
	public :
		MovingObject(std::string name, Position go_to,
					 MainIterator & mit, MainIndexer & min) :
				m_mit(mit), m_min(min) {
			m_name = name;
			m_go_to = go_to;
			dead = false;

			m_mit.collect(std::make_pair(this, 0));
		}

		virtual ~MovingObject() {
			std::cout << "R.I.P. me, " << m_name << std::endl;
		}

		virtual void declareTickForces(Collector<TickForce *> & force_collector) {
			std::cout << m_name << " déclare." << std::endl;
			force_collector.collect(new MoveForce(m_go_to, this));
			m_mit.collect(std::make_pair(this, 1));
		}

		virtual void addTickEffect(TickEffect * effect) {
			if( effect->getMetaData().isInstanceOf<DeathEffect>() ) {
				std::cout << "Diantre, moi, " << m_name << ", suis eu !" <<
				std::endl;
				dead = true;
			}
			delete effect;
		}

		virtual void applyTickEffects() {
			std::cout << m_name << " applique." << std::endl;
			if( dead ) {
				m_mit.discard(std::make_pair(this, 1));
				m_min.discard(this);
			}
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
	MoveForceCollector mvc;

	Foreman<> foreman(10);
	MainIndexer main_indexer;
	MainIterator main_iterator(foreman, main_indexer);
	main_iterator.collect(&mvc);

	/*MovingObject alice = MovingObject("Alice", Position::A);
	MovingObject bob = MovingObject("Bob", Position::B);
	MovingObject bob_2 = MovingObject("Bob2", Position::B);
	MovingObject charlie = MovingObject("Charlie", Position::C);*/

	main_indexer.collect(new MovingObject("Alice", Position::A,
						 main_iterator, main_indexer));
	main_indexer.collect(new MovingObject("Bob1", Position::B,
						 main_iterator, main_indexer));
	main_indexer.collect(new MovingObject("Bob2", Position::B,
						 main_iterator, main_indexer));
	main_indexer.collect(new MovingObject("Charlie", Position::C,
						 main_iterator, main_indexer));

	/*main_iterator.collect( std::make_pair(&alice, 0) );
	main_iterator.collect( std::make_pair(&bob, 0) );
	main_iterator.collect( std::make_pair(&bob_2, 0) );
	main_iterator.collect( std::make_pair(&charlie, 0) );*/

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
