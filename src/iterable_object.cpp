#include "iterable_object.h"

//TICKFORCE

TickForce::TickForce() {
	getMetaData().addClass<TickForce>();
}

TickForce::~TickForce() {}

//TICK EFFECT

TickEffect::TickEffect() {
	getMetaData().addClass<TickEffect>();
}

TickEffect::~TickEffect() {}

//FORCE DECLARATION WORK

ForceDeclarationWork::ForceDeclarationWork(
			IterableObject & subject,
			Collector<TickForce *> & force_collector,
			CallBack call_back) :
	Work(call_back),
	m_subject(subject),
	m_force_collector(force_collector) {}

void
ForceDeclarationWork::executeWork() {
	m_subject.declareTickForces(m_force_collector);
}

//EFFECTS APPLICATION WORK

EffectsApplicationWork::EffectsApplicationWork(
			IterableObject & subject,
			CallBack call_back) :
	Work(call_back),
	m_subject(subject) {}

void
EffectsApplicationWork::executeWork() {
	m_subject.applyTickEffects();
}

//ITERABLE OBJECT

IterableObject::IterableObject() {
	getMetaData().addClass<IterableObject>();
	m_force_declaration_work = NULL;
	m_effects_application_work = NULL;
	m_is_declared_modified = false;
	m_effect_declaration_lock = new MutexDataLock();
}

IterableObject::IterableObject(DataLock * lock) {
	getMetaData().addClass<IterableObject>();
	m_force_declaration_work = NULL;
	m_effects_application_work = NULL;
	m_is_declared_modified = false;
	m_effect_declaration_lock = lock;
}

IterableObject::~IterableObject() {
	delete m_effect_declaration_lock;
}

void IterableObject::declareTickForces(Collector<TickForce *> & force_collector) {}

void IterableObject::addTickEffect(TickEffect * effect) {}

void IterableObject::applyTickEffects() {}

void IterableObject::declareTickEffect(
		TickEffect * effect,
		std::list<IterableObject *> & declaration_list) {
	m_effect_declaration_lock->lock();
	addTickEffect(effect);
	if( ! m_is_declared_modified ) {
		m_is_declared_modified = true;
		declaration_list.push_back(this);
	}
	m_effect_declaration_lock->unlock();
}

Work &
IterableObject::forceDeclarationWork(Collector<TickForce *> & force_collector,
					 CallBack call_back) {
	clearWorks();
	m_force_declaration_work = new ForceDeclarationWork
		(*this, force_collector, call_back);
	return *m_force_declaration_work;
}

Work &
IterableObject::effectsApplicationWork(CallBack call_back) {
	m_effects_application_work = new EffectsApplicationWork
		(*this, call_back);
	return *m_effects_application_work;
}

void
IterableObject::clearWorks() {
	if( m_force_declaration_work ) {
		delete m_force_declaration_work;
		m_force_declaration_work = NULL;
	}
	if( m_effects_application_work ) {
		delete m_effects_application_work;
		m_effects_application_work = NULL;
	}
	m_is_declared_modified = false;
}

