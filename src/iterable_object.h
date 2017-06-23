#ifndef ITERABLE_OBJECT__H
#define ITERABLE_OBJECT__H
#include <list>
#include "metadata.h"
#include "collector.h"
#include "work.h"

class TickForce : public virtual MetaDataObject {
	public :
		TickForce();
		virtual ~TickForce();
};

class TickEffect : public virtual MetaDataObject {
	public :
		TickEffect();
		virtual ~TickEffect();
};

class IterableObject : public virtual MetaDataObject {
	protected :
		Work * m_force_declaration_work;
		Work * m_effects_application_work;
		bool m_is_declared_modified;
		DataLock * m_effect_declaration_lock;
	public :
		IterableObject();
		IterableObject(DataLock * data_lock);
		virtual ~IterableObject();

		virtual void declareTickForces(Collector<TickForce *> & force_collector);
		virtual void addTickEffect(TickEffect * effect);
		virtual void applyTickEffects();

		void declareTickEffect(
			TickEffect * effect,
			std::list<IterableObject *> & declaration_list);

		Work & forceDeclarationWork(
			Collector<TickForce *> & force_collector,
			CallBack call_back);
		Work & effectsApplicationWork(CallBack call_back);
		void clearWorks();
};

class ForceDeclarationWork : public Work {
	protected :
		IterableObject & m_subject;
		Collector<TickForce *> & m_force_collector;
	public :
		ForceDeclarationWork(
			IterableObject & subject,
			Collector<TickForce *> & force_collector,
			//EndWorkTrigger * end_trigger);
			CallBack call_back);
		virtual void executeWork();
};

class EffectsApplicationWork : public Work {
	protected :
		IterableObject & m_subject;
	public :
		EffectsApplicationWork(
			IterableObject & subject,
			//EndWorkTrigger * end_trigger);
			CallBack call_back);
		virtual void executeWork();
};

#endif
