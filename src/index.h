#ifndef INDEX__H
#define INDEX__H
#include <list>
#include <unordered_map>
#include "metadata.h"
#include "collector.h"
#include "provider.h"
#include "work.h"

class Indexer : public Provider<MetaDataObject *>,
					   ConditionalCollector<MetaDataObject *> {

	protected :
		//Filtre
		MetaData::Class m_filter;

		//Données
		MutexDataLock m_objects_lock;
		std::unordered_map<MetaDataObject *, MetaDataObject *> m_objects;
		std::list< Collector<MetaDataObject *> * > m_listeners;
	public :
		Indexer();
		virtual ~Indexer();
		Indexer(MetaData::Class filter);

		MetaData::Class getFilter();

		std::unordered_map<MetaDataObject *, MetaDataObject *> & getObjects();

		virtual bool doesAccept(MetaDataObject * object);

		virtual void collect(MetaDataObject * object);
		virtual void discard(MetaDataObject * object);

		virtual void addCollector(Collector<MetaDataObject *> & collector);
		virtual void removeCollector(Collector<MetaDataObject *> & collector);
};
/**
* Le travail du PhysicalIndexer est d'être le conteneur de tout les objets.
* Ce moteur est multi-thread, l'Indexer fournit donc une liste qui reste
* consistante aux modifications pendant chaque tick, n'effectuant les
* modifications qu'entre les tricks.
* On ne voudrait pas voir un objet désalloué au milieu d'un tick, alors que
* il doit encore faire du travail.
* Cela se fait grâce à une "flowgate", composée d'une liste d'entrée et une liste
* de sortie.
* Lorsque cette flowgate est activée, les objets entrant et sortant sont retenus
* dans les listes temporaires.
* Lorsque cette flowgate se désactive, tout les objets dans la liste de sortie
* sont retirés de l'indexer ; puis tout les objets dans la liste d'entrée sont
* rajoutés à l'indexer.
* Si la flowgate n'est pas activée, les objets entrent et sortent librement.
* La désactivation simultanée de toutes les flowgate de tout les indexers pouvant
* être couteuse, nous allons paralléliser ce calcul - un travail par indexer.
*/
class PhysicalIndexer : public Indexer {
	protected :
		//Flowgate
		std::atomic<bool> m_flowgate_activated;
		MutexDataLock m_enter_lock;
		std::list<MetaDataObject *> m_enter_list;
		MutexDataLock m_exit_lock;
		std::list<MetaDataObject *> m_exit_list;
	public :
		PhysicalIndexer();
		virtual ~PhysicalIndexer();

		virtual void collect(MetaDataObject * object);
		virtual void discard(MetaDataObject * object);

		virtual void switchFlowgate(bool activated);
};

class MainIndexer : virtual Collector<MetaDataObject *>,
					virtual Collector<Indexer *> {
	private :
		std::unordered_map<MetaData::Class, Indexer *> m_indexers;
		PhysicalIndexer m_physical_indexer;
	public :
		MainIndexer();
		virtual ~MainIndexer();

		virtual void collect(MetaDataObject * object);
		virtual void discard(MetaDataObject * object);

		virtual void collect(Indexer * object);
		virtual void discard(Indexer * object);

		virtual void switchFlowgate(bool activated);

		virtual Indexer * getIndexer(MetaData::Class filter);
};

#endif
