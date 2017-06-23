#include "metadata.h"


//MAINTEST

/*
#include <iostream>
int main(void) {
	A a;
	B b;
	C c;
	MetaDataObject & o1 = a;
	MetaDataObject & o2 = b;
	MetaDataObject & o3 = c;
	std::cout << "o1 (A) :" << std::endl;
	std::cout << "A? = " << o1.getMetaData().isInstanceOf<A>() << std::endl;
	std::cout << "B? = " << o1.getMetaData().isInstanceOf<B>() << std::endl;
	std::cout << "C? = " << o1.getMetaData().isInstanceOf<C>() << std::endl;
	std::cout << "o2 (B) :" << std::endl;
	std::cout << "A? = " << o2.getMetaData().isInstanceOf<A>() << std::endl;
	std::cout << "B? = " << o2.getMetaData().isInstanceOf<B>() << std::endl;
	std::cout << "C? = " << o2.getMetaData().isInstanceOf<C>() << std::endl;
	std::cout << "o3 (C) :" << std::endl;
	std::cout << "A? = " << o3.getMetaData().isInstanceOf<A>() << std::endl;
	std::cout << "B? = " << o3.getMetaData().isInstanceOf<B>() << std::endl;
	std::cout << "C? = " << o3.getMetaData().isInstanceOf<C>() << std::endl;
}
//*/

//METADATA

MetaData::MetaData() {
	m_classes = std::vector<std::type_index>();
}

MetaData::~MetaData() {}

//METADATAOBJET

MetaData & MetaDataObject::getMetaData() {
	return m_metadata;
}

MetaDataObject::MetaDataObject() {
	m_metadata = MetaData();
}

MetaDataObject::~MetaDataObject() {}
