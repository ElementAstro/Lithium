/*
 * destroy_detector.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-2-10

Description: Destroy Detector

*************************************************/

#ifndef ATOM_EVENT_DESTRORY_DETECTOR_HPP
#define ATOM_EVENT_DESTRORY_DETECTOR_HPP

#include <vector>
#include <memory>
#include <algorithm>

namespace Atom::Event
{
    class DestroyDetector
    {
    public:
        virtual ~DestroyDetector()
        {
            for (auto &c : checkers_)
            {
                c->destroyed_ = true;
            }
        }

        class Checker final
        {
        public:
            Checker() = default;
            Checker(DestroyDetector *dd)
            {
                auto it = std::find(dd->checkers_.begin(), dd->checkers_.end(), this);
                if (it == dd->checkers_.end())
                {
                    dd->checkers_.push_back(this);
                }
            }
            ~Checker()
            {
                auto it = std::find(dd_->checkers_.begin(), dd_->checkers_.end(), this);
                if (it != dd_->checkers_.end())
                {
                    dd_->checkers_.erase(it);
                }
            }
            bool isDestroyed() const
            {
                return destroyed_;
            }

        private:
            friend class DestroyDetector;

            DestroyDetector *dd_ = nullptr;
            bool destroyed_ = false;
        };

    protected:
        std::vector<Checker *> checkers_;
    };

#define DESTROY_DETECTOR_SETUP() \
    kev::DestroyDetector::Checker __dd_check(this);

#define DESTROY_DETECTOR_CHECK(ret) \
    if (__dd_check.isDestroyed())   \
        return ret;

#define DESTROY_DETECTOR_CHECK_VOID() DESTROY_DETECTOR_CHECK((void()))

} // namespace Atom::Event

#endif /* ATOM_EVENT_DESTRORY_DETECTOR */
