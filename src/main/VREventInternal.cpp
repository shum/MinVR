/** 
This file is part of the MinVR Open Source Project, which is developed and 
maintained collaboratively by the University of Minnesota and Brown University.

Copyright (c) 2016 Regents of the University of Minnesota and Brown University.
This software is distributed under the BSD-3 Clause license, which can be found
at: MinVR/LICENSE.txt.

Original Author(s) of this File: 
	Dan Keefe, 2016, University of Minnesota
	
Author(s) of Significant Updates/Modifications to the File:
	...
*/

#include "VREventInternal.h"
#include <api/VREvent.h>

namespace MinVR {

VREventInternal::VREventInternal(const std::string &name, VRDataIndex *dataIndex) :
  _name(name), _event(this), _dataIndex(dataIndex)
{

}

VREvent* VREventInternal::getAPIEvent() {
	return &_event;
}

VRDataIndex* VREventInternal::getDataIndex() {
	return _dataIndex;
}


}
