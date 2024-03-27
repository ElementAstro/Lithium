/*
 * basic.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-3-16

Description: Basic property class

*************************************************/

#include "basic.hpp"
#include "basic_p.hpp"

#include <cassert>

namespace Atom::Driver {

template <typename T>
PropertyBasicPrivateTemplate<T>::PropertyBasicPrivateTemplate(size_t count)
    : PropertyContainer<T>{*new PropertyView<T>()},
      PropertyPrivate(&this->typedProperty),
      raw{false},
      widgets(count) {
    this->typedProperty.setWidgets(widgets.data(), widgets.size());
}

template <typename T>
PropertyBasicPrivateTemplate<T>::PropertyBasicPrivateTemplate(
    RawPropertyType *rawProperty)
    : PropertyContainer<T>{*PropertyView<T>::cast(rawProperty)},
      PropertyPrivate(PropertyView<T>::cast(rawProperty)),
      raw{true} {}

template <typename T>
PropertyBasicPrivateTemplate<T>::~PropertyBasicPrivateTemplate() {
    if (!raw)
        delete &this->typedProperty;
}

template <typename T>
PropertyBasic<T>::~PropertyBasic() {}

template <typename T>
PropertyBasic<T>::PropertyBasic(PropertyBasicPrivate &dd) : Property(dd) {}

template <typename T>
PropertyBasic<T>::PropertyBasic(const std::shared_ptr<PropertyBasicPrivate> &dd)
    : Property(std::static_pointer_cast<PropertyPrivate>(dd)) {}

template <typename T>
void PropertyBasic<T>::setDeviceName(const std::string &name) {
    D_PTR(PropertyBasic);
    d->typedProperty.setDeviceName(name);
}

template <typename T>
void PropertyBasic<T>::setName(const std::string &name) {
    D_PTR(PropertyBasic);
    d->typedProperty.setName(name);
}

template <typename T>
void PropertyBasic<T>::setLabel(const std::string &label) {
    D_PTR(PropertyBasic);
    d->typedProperty.setLabel(label);
}

template <typename T>
void PropertyBasic<T>::setGroupName(const std::string &name) {
    D_PTR(PropertyBasic);
    d->typedProperty.setGroupName(name);
}

template <typename T>
void PropertyBasic<T>::setPermission(IPerm permission) {
    D_PTR(PropertyBasic);
    d->typedProperty.setPermission(permission);
}

template <typename T>
void PropertyBasic<T>::setTimeout(double timeout) {
    D_PTR(PropertyBasic);
    d->typedProperty.setTimeout(timeout);
}

template <typename T>
void PropertyBasic<T>::setState(IPState state) {
    D_PTR(PropertyBasic);
    d->typedProperty.setState(state);
}

template <typename T>
void PropertyBasic<T>::setTimestamp(const std::string &timestamp) {
    D_PTR(PropertyBasic);
    d->typedProperty.setTimestamp(timestamp);
}

template <typename T>
std::string PropertyBasic<T>::getDeviceName() const {
    D_PTR(const PropertyBasic);
    return d->typedProperty.getDeviceName();
}

template <typename T>
std::string PropertyBasic<T>::getName() const {
    D_PTR(const PropertyBasic);
    return d->typedProperty.getName();
}

template <typename T>
std::string PropertyBasic<T>::getLabel() const {
    D_PTR(const PropertyBasic);
    return d->typedProperty.getLabel();
}

template <typename T>
std::string PropertyBasic<T>::getGroupName() const {
    D_PTR(const PropertyBasic);
    return d->typedProperty.getGroupName();
}

template <typename T>
IPerm PropertyBasic<T>::getPermission() const {
    D_PTR(const PropertyBasic);
    return d->typedProperty.getPermission();
}

template <typename T>
std::string PropertyBasic<T>::getPermissionAsString() const {
    D_PTR(const PropertyBasic);
    return d->typedProperty.getPermissionAsString();
}

template <typename T>
double PropertyBasic<T>::getTimeout() const {
    D_PTR(const PropertyBasic);
    return d->typedProperty.getTimeout();
}

template <typename T>
IPState PropertyBasic<T>::getState() const {
    D_PTR(const PropertyBasic);
    return d->typedProperty.getState();
}

template <typename T>
std::string PropertyBasic<T>::getStateAsString() const {
    D_PTR(const PropertyBasic);
    return d->typedProperty.getStateAsString();
}

template <typename T>
std::string PropertyBasic<T>::getTimestamp() const {
    D_PTR(const PropertyBasic);
    return d->typedProperty.getTimestamp();
}

template <typename T>
bool PropertyBasic<T>::isEmpty() const {
    D_PTR(const PropertyBasic);
    return d->typedProperty.isEmpty();
}

template <typename T>
bool PropertyBasic<T>::isNameMatch(const std::string &otherName) const {
    D_PTR(const PropertyBasic);
    return d->typedProperty.isNameMatch(otherName);
}

template <typename T>
bool PropertyBasic<T>::isLabelMatch(const std::string &otherLabel) const {
    D_PTR(const PropertyBasic);
    return d->typedProperty.isLabelMatch(otherLabel);
}

template <typename T>
bool PropertyBasic<T>::load() {
    D_PTR(const PropertyBasic);
    return d->typedProperty.load();
}

template <typename T>
void PropertyBasic<T>::save(FILE *f) const {
    D_PTR(const PropertyBasic);
    d->typedProperty.save(f);
}

template <typename T>
void PropertyBasic<T>::vapply(std::string format, va_list args) const {
    D_PTR(const PropertyBasic);
    d->typedProperty.vapply(format, args);
}

template <typename T>
void PropertyBasic<T>::vdefine(std::string format, va_list args) const {
    D_PTR(const PropertyBasic);
    d->typedProperty.vdefine(format, args);
}

template <typename T>
void PropertyBasic<T>::apply(std::string format, ...) const {
    D_PTR(const PropertyBasic);
    va_list ap;
    va_start(ap, format);
    d->typedProperty.vapply(format, ap);
    va_end(ap);
}

template <typename T>
void PropertyBasic<T>::define(std::string format, ...) const {
    D_PTR(const PropertyBasic);
    va_list ap;
    va_start(ap, format);
    d->typedProperty.vdefine(format, ap);
    va_end(ap);
}

template <typename T>
void PropertyBasic<T>::apply() const {
    D_PTR(const PropertyBasic);
    d->typedProperty.apply();
}

template <typename T>
void PropertyBasic<T>::define() const {
    D_PTR(const PropertyBasic);
    d->typedProperty.define();
}

template <typename T>
WidgetView<T> *PropertyBasic<T>::findWidgetByName(std::string name) const {
    D_PTR(const PropertyBasic);
    return d->typedProperty.findWidgetByName(name);
}

template <typename T>
int PropertyBasic<T>::findWidgetIndexByName(std::string name) const {
    auto it = findWidgetByName(name);
    return int(it == nullptr ? -1 : it - begin());
}

template <typename T>
size_t PropertyBasic<T>::size() const {
    D_PTR(const PropertyBasic);
    return d->typedProperty.count();
}

template <typename T>
void PropertyBasic<T>::resize(size_t size) {
    D_PTR(PropertyBasic);
    assert(d->raw == false);
    d->widgets.resize(size);
    d->typedProperty.setWidgets(d->widgets.data(), d->widgets.size());
}

template <typename T>
void PropertyBasic<T>::reserve(size_t size) {
    D_PTR(PropertyBasic);
    assert(d->raw == false);
    d->widgets.reserve(size);
    d->typedProperty.setWidgets(d->widgets.data(), d->widgets.size());
}

template <typename T>
void PropertyBasic<T>::shrink_to_fit() {
    D_PTR(PropertyBasic);
    assert(d->raw == false);
    d->widgets.shrink_to_fit();
    d->typedProperty.setWidgets(d->widgets.data(), d->widgets.size());
}

template <typename T>
void PropertyBasic<T>::push(WidgetView<T> &&item) {
    D_PTR(PropertyBasic);
    assert(d->raw == false);
    item.setParent(&d->typedProperty);
    d->widgets.push_back(std::move(item));
    d->typedProperty.setWidgets(d->widgets.data(), d->widgets.size());
}

template <typename T>
void PropertyBasic<T>::push(const WidgetView<T> &item) {
    push(std::move(WidgetView<T>(item)));
}

template <typename T>
const WidgetView<T> *PropertyBasic<T>::at(size_t index) const {
    D_PTR(const PropertyBasic);
    return d->typedProperty.at(index);
}

template <typename T>
WidgetView<T> &PropertyBasic<T>::operator[](int index) const {
    D_PTR(const PropertyBasic);
    assert(index >= 0);
    return *d->typedProperty.at(index);
}

template <typename T>
WidgetView<T> *PropertyBasic<T>::begin() {
    D_PTR(PropertyBasic);
    return d->typedProperty.begin();
}

template <typename T>
WidgetView<T> *PropertyBasic<T>::end() {
    D_PTR(PropertyBasic);
    return d->typedProperty.end();
}

template <typename T>
const WidgetView<T> *PropertyBasic<T>::begin() const {
    D_PTR(const PropertyBasic);
    return d->typedProperty.begin();
}

template <typename T>
const WidgetView<T> *PropertyBasic<T>::end() const {
    D_PTR(const PropertyBasic);
    return d->typedProperty.end();
}

template <typename T>
PropertyView<T> *PropertyBasic<T>::operator&() {
    D_PTR(PropertyBasic);
    return &d->typedProperty;
}

template class PropertyBasicPrivateTemplate<IText>;
template class PropertyBasicPrivateTemplate<INumber>;
template class PropertyBasicPrivateTemplate<ISwitch>;
template class PropertyBasicPrivateTemplate<ILight>;
template class PropertyBasicPrivateTemplate<IBLOB>;

template class PropertyBasic<IText>;
template class PropertyBasic<INumber>;
template class PropertyBasic<ISwitch>;
template class PropertyBasic<ILight>;
template class PropertyBasic<IBLOB>;

}  // namespace Atom::Driver
