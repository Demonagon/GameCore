#ifndef METADATA__H
#define METADATA__H

#include <vector>
#include <typeindex>
#include <typeinfo>

class MetaData {
	private :
		std::vector<std::type_index> m_classes;
	public :
		typedef std::type_index Class;

		MetaData();
		virtual ~MetaData();

		std::vector<std::type_index> & getClasses() {
			return m_classes;
		}

		template<typename T> void addClass() {
			m_classes.push_back(MetaData::typeOf<T>());
		}

		bool isInstanceOf(Class c) {
			for(std::type_index s : m_classes)
				if(s == c) return true;
			return false;
		}

		template<typename T> bool isInstanceOf() {
			std::type_index c = MetaData::typeOf<T>();
			for(std::type_index s : m_classes)
				if(s == c) return true;
			return false;
		}

		template<typename T> static std::type_index typeOf(){
			return std::type_index( typeid( T() ) );
		}
};

class MetaDataObject {
	protected :
		MetaData m_metadata;
	public :
		MetaDataObject();
		virtual ~MetaDataObject();

		MetaData & getMetaData();
};

/*
class A : public virtual MetaDataObject {
	public :
		A() { getMetaData().addClass<A>(); }
};

class B : public virtual MetaDataObject {
	public :
		B() { getMetaData().addClass<B>(); }
};

class C : public A, B {
	public :
		C() { getMetaData().addClass<C>(); }
};//*/

#endif
