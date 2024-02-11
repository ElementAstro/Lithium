//
//  kmobject.h
//  kev
//
//  Created by Fengping Bao <jamol@live.com> on 7/17/16.
//  Copyright © 2016 kev. All rights reserved.
//

#ifndef __KMObject_H__
#define __KMObject_H__

#include <string>
#include <atomic>
#include <sstream>

namespace kev {

class KMObject
{
public:
    KMObject() {
        static std::atomic<long> s_objIdSeed{0};
        objId_ = ++s_objIdSeed;
    }
    
    const std::string& getObjKey() const {
        return objKey_;
    }
    
    long getObjId() const { return objId_; }
    
protected:
    std::string objKey_;
    long objId_ = 0;
};

#define KM_SetObjKey(x) \
do{ \
    std::stringstream ss; \
    ss<<x<<"_"<<objId_; \
    objKey_ = ss.str();\
}while(0)

} // namespace kev

#endif /* __KMObject_H__ */
