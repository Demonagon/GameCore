#ifndef ITERATION__H
#define ITERATION__H
#include <unordered_map>
#include <list>
#include "metadata.h"
#include "collector.h"
#include "work.h"
#include "index.h"
#include "iterable_object.h"

/**
* L'interface ForceCollector correspond à l'objet algorithmique qui calcule,
* à chaque tick, les implications des forces emises par les objets.
* Chaque ForceCollector collecte un type de force (d'où son nom).
* Un ForceCollector correspond généralement à une grande charge
* algorithmique dans l'ensemble du programme. Il est donc conseillé d'utiliser
* le parallélisme pour améliorer les performances. Pour cela, il faut déclarer
* d'un coup tout les travaux indépendants dans la fonction computingWork().
* Ces travaux ne garantissent pas l'ordre de leur exécution, et l'on ne peut donc
* pas se reposer sur un SleepLock pour attendre la fin de tout les travaux.
* Il est conseillé d'utiliser les CallBack internes aux work pour résoudre
* cet obstacle. La CallBack passée en paramètre doit toujours être appellée
* une seule et unique fois, lorsque tout les calculs d'effets sont terminés
* (et jamais avant).
*/
class ForceCollector : public ConditionalCollector<TickForce *> {
	protected :
		MetaData::Class m_filter;
		LambdaWork m_computing_work;
	public :
		ForceCollector(MetaData::Class filter);
		virtual ~ForceCollector();

		virtual MetaData::Class & getFilter();

		virtual bool doesAccept(TickForce * f);

		virtual Work & computingWork(std::list<IterableObject *> & modification_list,
							 CallBack call_back);

		virtual void compute(std::list<IterableObject *> & modification_list) = 0;
		virtual void clear() = 0;
		virtual int getWorkCount() = 0;
};

class PhysicalForceCollector : public ForceCollector {
	protected :
		std::list<TickForce *> m_forces;
	public :
		PhysicalForceCollector();
		virtual ~PhysicalForceCollector();

		virtual void collect(TickForce * force);
		virtual void discard(TickForce * force);
		virtual bool doesAccept(TickForce * force);

		void compute(std::list<IterableObject *> & modification_list);
		void clear();
		int getWorkCount();
};

class MainForceCollector : public Collector<TickForce *>,
								  Collector<ForceCollector *> {
	protected :
		std::unordered_map<MetaData::Class, ForceCollector *> m_collectors;
		PhysicalForceCollector m_physical_collector;
	public :
		MainForceCollector();
		virtual ~MainForceCollector();

		virtual void collect(TickForce * force);
		virtual void discard(TickForce * force);

		virtual void collect(ForceCollector * collector);
		virtual void discard(ForceCollector * collector);

		int getWorksCount();

		void declareWorks(Collector<Work &> & collector,
						  std::list<IterableObject *> & modification_list,
						  CallBack call_back);

		void clear();
};

/**
* Classe qui retient qui doit être exécuté, et quand. C'est le travail de
* l'itérable de se replacer dans la table, par exemple un object qui désire
* être exécuté tout les ticks, ou un tick sur deux...
* Note : delay = 0 signifie exécuter l'objet à la prochaine itération.
* LockFront() permet d'empêcher de nouveaux travaux de s'ajouter à la liste
* pendant que l'on execute cette dernière. Ces travaux sont décalé d'un vers
* la droite pendant la durée de l'opération. <- en cours d'étude
*/
class IterationTable : public Collector< std::pair<IterableObject *, int > > {
	protected :
		std::list< std::list< IterableObject * > > m_table;
		//std::atomic<bool> m_front_lock;
	public :
		IterationTable();

		virtual void createSpace(int delay);
		virtual void insert(std::pair<IterableObject *, int> execution);
		virtual void collect(std::pair<IterableObject *, int> execution);
		virtual void discard(std::pair<IterableObject *, int> execution);

		//int operatingOffSet();

		//void lockFront(bool state);
		bool has_a_front();
		std::list< IterableObject * > & front();
		void pop_front();
};

/**
* Pour calculer un tick, le main iterator :
1) Récupère la liste des objets actifs ce tick (dans la table d'itération)
2) Ajoute des travaux au contremaître pour chaque object actif, afin
	de lui faire déclarer les forces qu'il implique ce tick
3) S'endort sur un SleepLock le temps que ces travaux soient tous achevés
4) Ajoute des travaux au contremaître pour chaque ForceCollector une fois
	reveillé
5) S'endort sur le même SleepLock le temps que ces travaux soient tous achevés
6) Ajoute des travaux au contemaître pour chaque objet affecté par les effets
7) S'endort jusqu'à ce que tout ces travaux soient achevés
*/
class MainIterator : public virtual Collector<ForceCollector *>,
							virtual Collector< std::pair<IterableObject*,int> >{
	protected :
		MainForceCollector m_force_collector;
		IterationTable m_iteration_table;
		Foreman<> & m_foreman;
		MainIndexer & m_indexer;
	public :
		MainIterator(Foreman<> & foreman, MainIndexer & indexer);
		virtual ~MainIterator();

		void tick();

		void declareForces();
		void computeEffects(std::list<IterableObject *> & modified_objects);
		void applyEffects(std::list<IterableObject *> & modified_objects);

		virtual void collect(ForceCollector * collector);
		virtual void discard(ForceCollector * collector);
		virtual void collect(std::pair<IterableObject *, int> execution);
		virtual void discard(std::pair<IterableObject *, int> execution);
		
};

#endif
