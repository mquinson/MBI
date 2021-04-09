/* This file is part of GTI (Generic Tool Infrastructure)
 *
 * Copyright (C)
 *  2008-2019 ZIH, Technische Universitaet Dresden, Federal Republic of Germany
 *  2008-2019 Lawrence Livermore National Laboratories, United States of America
 *  2013-2019 RWTH Aachen University, Federal Republic of Germany
 *
 * See the LICENSE file in the package base directory for details
 */

/**
 * @file TestChannelId.cpp
 *       Test driver to test functionality of GtiChannelId.
 *
 * @author Tobias Hilbrich
 * @date 7.12.2010
 */

#include <iostream>
#include <map>

#include "GtiChannelId.h"

using namespace gti;

int main (int argc, char** argv)
{
	//create two ids
	GtiChannelId<4,5,20> a (5), b (5);

	//initialize to 0
	a.set64(0,0);
	a.set64(1,0);
	a.set64(2,0);
	a.set64(3,0);
	b.set64(0,0);
	b.set64(1,0);
	b.set64(2,0);
	b.set64(3,0);

	//set sub-ids (1048574 = 0xFFFFE, tests whether really all bits are set to 1 (the last E becomes F as channel id in storage is incremented by one))
	// a: 0.1048574.0.1048574.1024
	// b: 1048574.0.1048574.1024.1048574
	std::cout << "Before setting values: A={" << a.toString() << "} B={" << b.toString() << "}" << std::endl;

	a.setSubId(0,1024);
	a.setSubId(1,1048574);
	a.setSubId(2,0);
	a.setSubId(3,1048574);
	a.setSubId(4,0);
	a.setSubIdNumChannels(0,0xFFFFE);
	a.setSubIdNumChannels(1,1024);
	//2 unset !!
	a.setSubIdNumChannels(3,0);
	a.setSubIdNumChannels(4,0xFFFF0);

	b.setSubId(0,1048574);
	b.setSubId(1,1024);
	b.setSubId(2,1048574);
	b.setSubId(3,0);
	b.setSubId(4,1048574);

	std::cout << "A={" << a.toString() << "} B={" << b.toString() << "}" << std::endl;
	std::cout << "NumChannels: A:" << a.getSubIdNumChannels(0) << ";" << a.getSubIdNumChannels(1) << ";" << a.getSubIdNumChannels(2) << ";" << a.getSubIdNumChannels(3) << ";" << a.getSubIdNumChannels(4) << std::endl;

	//Test the interface
    I_ChannelId 	*c = a.copy (),
    						*d = b.copy (),
    						*e = c->copy ();

    std::cout << "C={" << c->toString() << "} D={" << d->toString() << "} (copies of A and B)" << std::endl;
    std::cout << " a < b = " << (c->isLessThan(d)) << std::endl;
    std::cout << " b < a = " << (d->isLessThan(c)) << std::endl;
    std::cout << " a == a = " << (e->isEqual(c)) << std::endl;

    std::map<I_ChannelId*, bool, I_ChannelIdComp> chanMap;
    std::map<I_ChannelId*, bool, I_ChannelIdComp>::iterator iter;
    chanMap.insert(std::make_pair(c,true));
    chanMap.insert(std::make_pair(d,true));
    chanMap.insert(std::make_pair(e,true));

    std::cout << "Pushed 3 channel ids into map, two of them were equal, here is the maps content:" << std::endl;
    for (iter = chanMap.begin(); iter != chanMap.end(); iter++)
    {
    		I_ChannelId* id = iter->first;
    		std::cout << id->toString() << std::endl;
    }

	return 0;
}
