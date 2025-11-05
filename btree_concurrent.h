#ifndef __BTREE_CONCURRENT_H__
#define __BTREE_CONCURRENT_H__
#include <shared_mutex>
#include <mutex>
#include <memory>
#include <vector>
#include <atomic>
#include "btree.h"
template <typename Trait> class BatchOperations;
template <typename Trait>
class ThreadSafeBTree
{
       typedef typename Trait::keyType    keyType;
       typedef typename Trait::ObjIDType  ObjIDType;
       typedef typename Trait::CompareFn  CompareFn;
       typedef typename BTree<Trait>::ObjectInfo ObjectInfo;
public:
       explicit ThreadSafeBTree(size_t order = DEFAULT_BTREE_ORDER, bool unique = true)
               : m_Tree(order, unique), m_ReadCount(0), m_WriteCount(0)
       {}
       ~ThreadSafeBTree() = default;
       ThreadSafeBTree(const ThreadSafeBTree&) = delete;
       ThreadSafeBTree& operator=(const ThreadSafeBTree&) = delete;
       ThreadSafeBTree(ThreadSafeBTree&& other) noexcept
               : m_Tree(std::move(other.m_Tree)),
                 m_ReadCount(other.m_ReadCount.load()),
                 m_WriteCount(other.m_WriteCount.load())
       {}
       ThreadSafeBTree& operator=(ThreadSafeBTree&& other) noexcept {
               if(this != &other) {
                       std::unique_lock<std::shared_mutex> lock1(m_Mutex, std::defer_lock);
                       std::unique_lock<std::shared_mutex> lock2(other.m_Mutex, std::defer_lock);
                       std::lock(lock1, lock2);
                       m_Tree = std::move(other.m_Tree);
                       m_ReadCount = other.m_ReadCount.load();
                       m_WriteCount = other.m_WriteCount.load();
               }
               return *this;
       }
       bool Insert(const keyType key, const ObjIDType ObjID) {
               std::unique_lock<std::shared_mutex> lock(m_Mutex);
               m_WriteCount++;
               return m_Tree.Insert(key, ObjID);
       }
       bool Remove(const keyType key, const ObjIDType ObjID) {
               std::unique_lock<std::shared_mutex> lock(m_Mutex);
               m_WriteCount++;
               return m_Tree.Remove(key, ObjID);
       }
       ObjIDType Search(const keyType key) const {
               std::shared_lock<std::shared_mutex> lock(m_Mutex);
               m_ReadCount++;
               return m_Tree.Search(key);
       }
       size_t size() const {
               std::shared_lock<std::shared_mutex> lock(m_Mutex);
               return m_Tree.size();
       }
       size_t height() const {
               std::shared_lock<std::shared_mutex> lock(m_Mutex);
               return m_Tree.height();
       }
       size_t GetOrder() const {
               std::shared_lock<std::shared_mutex> lock(m_Mutex);
               return m_Tree.GetOrder();
       }
       bool Write(const std::string& filename) const {
               std::shared_lock<std::shared_mutex> lock(m_Mutex);
               return m_Tree.Write(filename);
       }
       bool Read(const std::string& filename) {
               std::unique_lock<std::shared_mutex> lock(m_Mutex);
               return m_Tree.Read(filename);
       }
       template <typename Function, typename... Args>
       void ForEach(Function&& func, Args&&... args) const {
               std::shared_lock<std::shared_mutex> lock(m_Mutex);
               m_ReadCount++;
               m_Tree.ForEach(std::forward<Function>(func), std::forward<Args>(args)...);
       }
       template <typename Predicate, typename... Args>
       ObjectInfo* FirstThat(Predicate&& pred, Args&&... args) {
               std::shared_lock<std::shared_mutex> lock(m_Mutex);
               m_ReadCount++;
               return m_Tree.FirstThat(std::forward<Predicate>(pred), std::forward<Args>(args)...);
       }
       template <typename Predicate, typename... Args>
       void FindAll(std::vector<ObjectInfo*>& results, Predicate&& pred, Args&&... args) {
               std::shared_lock<std::shared_mutex> lock(m_Mutex);
               m_ReadCount++;
               m_Tree.FindAll(results, std::forward<Predicate>(pred), std::forward<Args>(args)...);
       }
       template <typename Predicate, typename... Args>
       size_t CountIf(Predicate&& pred, Args&&... args) const {
               std::shared_lock<std::shared_mutex> lock(m_Mutex);
               m_ReadCount++;
               return m_Tree.CountIf(std::forward<Predicate>(pred), std::forward<Args>(args)...);
       }
       template <typename Predicate, typename... Args>
       bool AnyOf(Predicate&& pred, Args&&... args) const {
               std::shared_lock<std::shared_mutex> lock(m_Mutex);
               m_ReadCount++;
               return m_Tree.AnyOf(std::forward<Predicate>(pred), std::forward<Args>(args)...);
       }
       template <typename Predicate, typename... Args>
       bool AllOf(Predicate&& pred, Args&&... args) const {
               std::shared_lock<std::shared_mutex> lock(m_Mutex);
               m_ReadCount++;
               return m_Tree.AllOf(std::forward<Predicate>(pred), std::forward<Args>(args)...);
       }
       template <typename Predicate, typename... Args>
       bool NoneOf(Predicate&& pred, Args&&... args) const {
               std::shared_lock<std::shared_mutex> lock(m_Mutex);
               m_ReadCount++;
               return m_Tree.NoneOf(std::forward<Predicate>(pred), std::forward<Args>(args)...);
       }
       template <typename T, typename BinaryOp, typename... Args>
       T Accumulate(T init, BinaryOp&& op, Args&&... args) const {
               std::shared_lock<std::shared_mutex> lock(m_Mutex);
               m_ReadCount++;
               return m_Tree.Accumulate(init, std::forward<BinaryOp>(op), std::forward<Args>(args)...);
       }
       template <typename OutputContainer, typename UnaryOp, typename... Args>
       void Transform(OutputContainer& output, UnaryOp&& op, Args&&... args) const {
               std::shared_lock<std::shared_mutex> lock(m_Mutex);
               m_ReadCount++;
               m_Tree.Transform(output, std::forward<UnaryOp>(op), std::forward<Args>(args)...);
       }
       class SnapshotIterator {
       public:
               using iterator_category = std::forward_iterator_tag;
               using value_type        = ObjectInfo;
               using difference_type   = std::ptrdiff_t;
               using pointer           = const ObjectInfo*;
               using reference         = const ObjectInfo&;
       private:
               std::vector<ObjectInfo> m_Snapshot;
               size_t                  m_Index;
               friend class ThreadSafeBTree<Trait>;
               explicit SnapshotIterator(const ThreadSafeBTree* tree, bool isEnd)
                       : m_Index(0)
               {
                       if(!isEnd && tree) {
                               std::shared_lock<std::shared_mutex> lock(tree->m_Mutex);
                               tree->m_Tree.ForEach([this](auto& info, size_t) {
                                       m_Snapshot.push_back(info);
                               });
                       }
               }
       public:
               SnapshotIterator() : m_Index(0) {}
               reference operator*() const {
                       return m_Snapshot[m_Index];
               }
               pointer operator->() const {
                       return &m_Snapshot[m_Index];
               }
               SnapshotIterator& operator++() {
                       if(m_Index < m_Snapshot.size()) ++m_Index;
                       return *this;
               }
               SnapshotIterator operator++(int) {
                       SnapshotIterator tmp = *this;
                       if(m_Index < m_Snapshot.size()) ++m_Index;
                       return tmp;
               }
               bool operator==(const SnapshotIterator& other) const {
                       if(m_Index >= m_Snapshot.size() && other.m_Index >= other.m_Snapshot.size())
                               return true;
                       if(m_Index >= m_Snapshot.size() || other.m_Index >= other.m_Snapshot.size())
                               return false;
                       return m_Index == other.m_Index &&
                              m_Snapshot.size() == other.m_Snapshot.size();
               }
               bool operator!=(const SnapshotIterator& other) const {
                       return !(*this == other);
               }
       };
       using iterator       = SnapshotIterator;
       using const_iterator = SnapshotIterator;
       iterator begin() {
               return SnapshotIterator(this, false);
       }
       iterator end() {
               return SnapshotIterator(this, true);
       }
       const_iterator begin() const {
               return SnapshotIterator(this, false);
       }
       const_iterator end() const {
               return SnapshotIterator(this, true);
       }
       const_iterator cbegin() const {
               return SnapshotIterator(this, false);
       }
       const_iterator cend() const {
               return SnapshotIterator(this, true);
       }
       uint64_t GetReadCount() const {
               return m_ReadCount.load();
       }
       uint64_t GetWriteCount() const {
               return m_WriteCount.load();
       }
       void ResetCounters() {
               m_ReadCount = 0;
               m_WriteCount = 0;
       }
private:
       friend class BatchOperations<Trait>;
       BTree<Trait>               m_Tree;
       mutable std::shared_mutex  m_Mutex;
       mutable std::atomic<uint64_t> m_ReadCount;
       mutable std::atomic<uint64_t> m_WriteCount;
};
template <typename Trait>
class ReadWriteLockGuard {
public:
       explicit ReadWriteLockGuard(std::shared_mutex& mutex, bool write_lock)
               : m_Mutex(mutex), m_WriteLock(write_lock)
       {
               if(m_WriteLock) {
                       m_Mutex.lock();
               } else {
                       m_Mutex.lock_shared();
               }
       }
       ~ReadWriteLockGuard() {
               if(m_WriteLock) {
                       m_Mutex.unlock();
               } else {
                       m_Mutex.unlock_shared();
               }
       }
       ReadWriteLockGuard(const ReadWriteLockGuard&) = delete;
       ReadWriteLockGuard& operator=(const ReadWriteLockGuard&) = delete;
private:
       std::shared_mutex& m_Mutex;
       bool               m_WriteLock;
};
template <typename Trait>
class BatchOperations {
public:
       explicit BatchOperations(ThreadSafeBTree<Trait>& tree)
               : m_Tree(tree), m_Lock(tree.m_Mutex)
       {}
       bool Insert(const typename Trait::keyType key, const typename Trait::ObjIDType ObjID) {
               return m_Tree.m_Tree.Insert(key, ObjID);
       }
       bool Remove(const typename Trait::keyType key, const typename Trait::ObjIDType ObjID) {
               return m_Tree.m_Tree.Remove(key, ObjID);
       }
       typename Trait::ObjIDType Search(const typename Trait::keyType key) const {
               return m_Tree.m_Tree.Search(key);
       }
private:
       ThreadSafeBTree<Trait>&              m_Tree;
       std::unique_lock<std::shared_mutex>  m_Lock;
};
#endif