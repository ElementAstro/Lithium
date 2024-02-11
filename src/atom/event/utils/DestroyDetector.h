//
//  DestroyDetector.h
//  kev
//
//  Created by Fengping Bao <jamol@live.com> on 8/9/16.
//  Copyright © 2016-2020 kev. All rights reserved.
//

#ifndef __DestroyDetector_H__
#define __DestroyDetector_H__


namespace kev {

class DestroyDetector
{
public:
    virtual ~DestroyDetector()
    {
        for (auto *c = head_.next_; c != &head_; c = c->next_) {
            c->destroyed_ = true;
        }
    }
    
    class Checker final
    {
    public:
        Checker() = default;
        Checker(DestroyDetector *dd)
        {
            prev_ = &dd->head_;
            next_ = prev_->next_;
            next_->prev_ = this;
            prev_->next_ = this;
        }
        ~Checker()
        {
            prev_->next_ = next_;
            next_->prev_ = prev_;
            prev_ = next_ = this;
        }
        bool isDestroyed() const
        {
            return destroyed_;
        }
        
    protected:
        friend class DestroyDetector;
        
        bool destroyed_ = false;
        Checker* prev_ = this;
        Checker* next_ = this;
    };

protected:
    Checker head_;
};

#define DESTROY_DETECTOR_SETUP() \
    kev::DestroyDetector::Checker __dd_check(this);

#define DESTROY_DETECTOR_CHECK(ret) \
    if(__dd_check.isDestroyed()) return ret;

#define DESTROY_DETECTOR_CHECK_VOID() DESTROY_DETECTOR_CHECK((void()))

} // namespace kev

#endif /* __DestroyDetector_H__ */
