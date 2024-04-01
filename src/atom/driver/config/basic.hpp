/*
 * basic.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-3-16

Description: Basic property class

*************************************************/

#ifndef ATOM_DRIVER_PROPERTY_BASIC_HPP
#define ATOM_DRIVER_PROPERTY_BASIC_HPP

#include <algorithm>
#include "macros.hpp"
#include "property.hpp"


namespace Atom::Driver {

using WidgetText = Atom::Driver::WidgetViewText;
using WidgetNumber = Atom::Driver::WidgetViewNumber;
using WidgetSwitch = Atom::Driver::WidgetViewSwitch;
using WidgetLight = Atom::Driver::WidgetViewLight;
using WidgetBlob = Atom::Driver::WidgetViewBlob;

template <typename>
class PropertyBasicPrivateTemplate;

template <typename T>
class PropertyBasic : public Atom::Driver::Property {
    using PropertyBasicPrivate = PropertyBasicPrivateTemplate<T>;
    DECLARE_PRIVATE(PropertyBasic)
public:
    using ViewType = T;

public:
    ~PropertyBasic();

public:
    void setDeviceName(const std::string &name);
    void setName(const std::string &name);
    void setLabel(const std::string &label);
    void setGroupName(const std::string &name);
    void setPermission(IPerm permission);
    void setTimeout(double timeout);
    void setState(IPState state);

    void setTimestamp(const std::string &timestamp);

public:
    std::string getDeviceName() const;
    std::string getName() const;
    std::string getLabel() const;
    std::string getGroupName() const;

    IPerm getPermission() const;
    std::string getPermissionAsString() const;

    double getTimeout() const;
    IPState getState() const;
    std::string getStateAsString() const;

    std::string getTimestamp() const;

public:
    bool isEmpty() const;

    bool isNameMatch(const std::string &otherName) const;

    bool isLabelMatch(const std::string &otherLabel) const;

public:
    /**
     * @brief load Attempt to load property values from configuration file.
     * @return True if value was read successfully from file, false otherwise.
     */
    bool load();

    /**
     * @brief save Save property to configuration file.
     * @param f Pointer to existing open configuration file.
     */
    void save(FILE *f) const;

    void vapply(std::string format, va_list args) const;
    void vdefine(std::string format, va_list args) const;

    void apply(std::string format, ...) const ATTRIBUTE_FORMAT_PRINTF(2, 3);
    void define(std::string format, ...) const ATTRIBUTE_FORMAT_PRINTF(2, 3);

    void apply() const;
    void define() const;

protected:
    PropertyView<T> *operator&();

public:
    size_t size() const;
    size_t count() const { return size(); }

public:
    void reserve(size_t size);
    void resize(size_t size);

    void shrink_to_fit();

    void push(WidgetView<T> &&item);
    void push(const WidgetView<T> &item);

    const WidgetView<T> *at(size_t index) const;

    WidgetView<T> &operator[](int index) const;

public:  // STL-style iterators
    WidgetView<T> *begin();
    WidgetView<T> *end();
    const WidgetView<T> *begin() const;
    const WidgetView<T> *end() const;

    template <typename Predicate>
    WidgetView<T> *find_if(Predicate pred) {
        return std::find_if(begin(), end(), pred);
    }

    template <typename Predicate>
    const WidgetView<T> *find_if(Predicate pred) const {
        return std::find_if(begin(), end(), pred);
    }

public:
    WidgetView<T> *findWidgetByName(std::string name) const;
    int findWidgetIndexByName(std::string name) const;

protected:
    PropertyBasic(PropertyBasicPrivate &dd);
    PropertyBasic(const std::shared_ptr<PropertyBasicPrivate> &dd);
};

}  // namespace Atom::Driver

#endif
