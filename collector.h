#ifndef TEST1_COLLECTOR_H
#define TEST1_COLLECTOR_H

#include <mutex>
#include <vector>


namespace collector {

    template<class Element>
    class VectorCollector {
    public:
        typedef std::vector<Element> collection;

        void operator()(Element &&path) {
            std::lock_guard guard{mutex};
            items.push_back(path);
        }

        collection retrieveAndClear() {
            std::lock_guard guard{mutex};
            collection result = std::move(items);
            // Most standard library implementations will leave the source collection
            // empty after a move, but this is not guaranteed, so clear it explicitly.
            items.clear();
            return result;
        }

    private:
        std::mutex mutex;
        collection items;
    };

}

#endif //TEST1_COLLECTOR_H
