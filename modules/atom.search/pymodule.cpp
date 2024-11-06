#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "atom/search/cache.hpp"
#include "atom/search/lru.hpp"
#include "atom/search/search.hpp"

namespace py = pybind11;
using namespace atom::search;

template <typename T>
void bind_resource_cache(py::module &m, const std::string &name) {
    py::class_<ResourceCache<T>>(m, name.c_str())
        .def(py::init<int>(), "Constructor", py::arg("max_size"))
        .def("insert", &ResourceCache<T>::insert,
             "Insert a resource into the cache with an expiration time",
             py::arg("key"), py::arg("value"), py::arg("expiration_time"))
        .def("contains", &ResourceCache<T>::contains,
             "Check if the cache contains a resource with the specified key",
             py::arg("key"))
        .def("get", &ResourceCache<T>::get,
             "Retrieve a resource from the cache", py::arg("key"))
        .def("remove", &ResourceCache<T>::remove,
             "Remove a resource from the cache", py::arg("key"))
        .def("async_get", &ResourceCache<T>::asyncGet,
             "Asynchronously retrieve a resource from the cache",
             py::arg("key"))
        .def("async_insert", &ResourceCache<T>::asyncInsert,
             "Asynchronously insert a resource into the cache with an "
             "expiration time",
             py::arg("key"), py::arg("value"), py::arg("expiration_time"))
        .def("clear", &ResourceCache<T>::clear,
             "Clear all resources from the cache")
        .def("size", &ResourceCache<T>::size,
             "Get the number of resources in the cache")
        .def("empty", &ResourceCache<T>::empty, "Check if the cache is empty")
        .def("evict_oldest", &ResourceCache<T>::evictOldest,
             "Evict the oldest resource from the cache")
        .def("is_expired", &ResourceCache<T>::isExpired,
             "Check if a resource with the specified key is expired",
             py::arg("key"))
        .def("async_load", &ResourceCache<T>::asyncLoad,
             "Asynchronously load a resource into the cache using a provided "
             "function",
             py::arg("key"), py::arg("load_data_function"))
        .def("set_max_size", &ResourceCache<T>::setMaxSize,
             "Set the maximum size of the cache", py::arg("max_size"))
        .def("set_expiration_time", &ResourceCache<T>::setExpirationTime,
             "Set the expiration time for a resource in the cache",
             py::arg("key"), py::arg("expiration_time"))
        .def("read_from_file", &ResourceCache<T>::readFromFile,
             "Read resources from a file and insert them into the cache",
             py::arg("file_path"), py::arg("deserializer"))
        .def("write_to_file", &ResourceCache<T>::writeToFile,
             "Write the resources in the cache to a file", py::arg("file_path"),
             py::arg("serializer"))
        .def("remove_expired", &ResourceCache<T>::removeExpired,
             "Remove expired resources from the cache")
        .def("read_from_json_file", &ResourceCache<T>::readFromJsonFile,
             "Read resources from a JSON file and insert them into the cache",
             py::arg("file_path"), py::arg("from_json"))
        .def("write_to_json_file", &ResourceCache<T>::writeToJsonFile,
             "Write the resources in the cache to a JSON file",
             py::arg("file_path"), py::arg("to_json"))
        .def("insert_batch", &ResourceCache<T>::insertBatch,
             "Insert multiple resources into the cache with an expiration time",
             py::arg("items"), py::arg("expiration_time"))
        .def("remove_batch", &ResourceCache<T>::removeBatch,
             "Remove multiple resources from the cache", py::arg("keys"))
        .def("on_insert", &ResourceCache<T>::onInsert,
             "Register a callback to be called on insertion",
             py::arg("callback"))
        .def("on_remove", &ResourceCache<T>::onRemove,
             "Register a callback to be called on removal", py::arg("callback"))
        .def("get_statistics", &ResourceCache<T>::getStatistics,
             "Retrieve cache statistics");
}

template <typename Key, typename Value>
void bind_thread_safe_lru_cache(py::module &m, const std::string &name) {
    py::class_<ThreadSafeLRUCache<Key, Value>>(m, name.c_str())
        .def(py::init<size_t>(), "Constructor", py::arg("max_size"))
        .def("get", &ThreadSafeLRUCache<Key, Value>::get,
             "Retrieve a value from the cache", py::arg("key"))
        .def("put", &ThreadSafeLRUCache<Key, Value>::put,
             "Insert or update a value in the cache", py::arg("key"),
             py::arg("value"), py::arg("ttl") = std::nullopt)
        .def("erase", &ThreadSafeLRUCache<Key, Value>::erase,
             "Erase an item from the cache", py::arg("key"))
        .def("clear", &ThreadSafeLRUCache<Key, Value>::clear,
             "Clear all items from the cache")
        .def("keys", &ThreadSafeLRUCache<Key, Value>::keys,
             "Retrieve all keys in the cache")
        .def("pop_lru", &ThreadSafeLRUCache<Key, Value>::popLru,
             "Remove and return the least recently used item")
        .def("resize", &ThreadSafeLRUCache<Key, Value>::resize,
             "Resize the cache to a new maximum size", py::arg("new_max_size"))
        .def("size", &ThreadSafeLRUCache<Key, Value>::size,
             "Get the current size of the cache")
        .def("load_factor", &ThreadSafeLRUCache<Key, Value>::loadFactor,
             "Get the current load factor of the cache")
        .def("set_insert_callback",
             &ThreadSafeLRUCache<Key, Value>::setInsertCallback,
             "Set the callback function to be called when a new item is "
             "inserted",
             py::arg("callback"))
        .def("set_erase_callback",
             &ThreadSafeLRUCache<Key, Value>::setEraseCallback,
             "Set the callback function to be called when an item is erased",
             py::arg("callback"))
        .def("set_clear_callback",
             &ThreadSafeLRUCache<Key, Value>::setClearCallback,
             "Set the callback function to be called when the cache is cleared",
             py::arg("callback"))
        .def("hit_rate", &ThreadSafeLRUCache<Key, Value>::hitRate,
             "Get the hit rate of the cache")
        .def("save_to_file", &ThreadSafeLRUCache<Key, Value>::saveToFile,
             "Save the cache contents to a file", py::arg("filename"))
        .def("load_from_file", &ThreadSafeLRUCache<Key, Value>::loadFromFile,
             "Load cache contents from a file", py::arg("filename"));
}

PYBIND11_MODULE(search, m) {
    m.doc() = "Search engine module";

    bind_resource_cache<std::string>(m, "StringResourceCache");
    bind_resource_cache<int>(m, "IntResourceCache");
    bind_resource_cache<double>(m, "DoubleResourceCache");

    bind_thread_safe_lru_cache<std::string, std::string>(m, "StringLRUCache");
    bind_thread_safe_lru_cache<int, int>(m, "IntLRUCache");
    bind_thread_safe_lru_cache<int, double>(m, "IntDoubleLRUCache");
    bind_thread_safe_lru_cache<int, std::string>(m, "IntStringLRUCache");
    bind_thread_safe_lru_cache<std::string, int>(m, "StringIntLRUCache");
    bind_thread_safe_lru_cache<std::string, double>(m, "StringDoubleLRUCache");

    py::register_exception<DocumentNotFoundException>(
        m, "DocumentNotFoundException");

    py::class_<Document>(m, "Document")
        .def(py::init<std::string, std::string,
                      std::initializer_list<std::string>>(),
             py::arg("id"), py::arg("content"), py::arg("tags"))
        .def_readwrite("id", &Document::id)
        .def_readwrite("content", &Document::content)
        .def_readwrite("tags", &Document::tags)
        .def_readwrite("click_count", &Document::clickCount);

    py::class_<SearchEngine>(m, "SearchEngine")
        .def(py::init<>())
        .def("add_document", &SearchEngine::addDocument,
             "Add a document to the search engine", py::arg("doc"))
        .def("remove_document", &SearchEngine::removeDocument,
             "Remove a document from the search engine", py::arg("doc_id"))
        .def("update_document", &SearchEngine::updateDocument,
             "Update an existing document in the search engine", py::arg("doc"))
        .def("search_by_tag", &SearchEngine::searchByTag,
             "Search for documents by a specific tag", py::arg("tag"))
        .def("fuzzy_search_by_tag", &SearchEngine::fuzzySearchByTag,
             "Perform a fuzzy search for documents by a tag with a specified "
             "tolerance",
             py::arg("tag"), py::arg("tolerance"))
        .def("search_by_tags", &SearchEngine::searchByTags,
             "Search for documents by multiple tags", py::arg("tags"))
        .def("search_by_content", &SearchEngine::searchByContent,
             "Search for documents by content", py::arg("query"))
        .def("boolean_search", &SearchEngine::booleanSearch,
             "Perform a boolean search for documents by a query",
             py::arg("query"))
        .def("auto_complete", &SearchEngine::autoComplete,
             "Provide autocomplete suggestions for a given prefix",
             py::arg("prefix"))
        .def("save_index", &SearchEngine::saveIndex,
             "Save the current index to a file", py::arg("filename"))
        .def("load_index", &SearchEngine::loadIndex,
             "Load the index from a file", py::arg("filename"));
}